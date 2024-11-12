// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_SINGLE_FILTER_HPP
#define ZLEQUALIZER_SINGLE_FILTER_HPP

#include <juce_dsp/juce_dsp.h>
#include "../filter_design/filter_design.hpp"
#include "coeff/martin_coeff.hpp"
#include "iir_base.hpp"
#include "svf_base.hpp"

namespace zlFilter {
    /**
     * an IIR filter which processes audio on the real-time thread
     * the maximum modulation rate of parameters is once per block
     * @tparam FloatType the float type of input audio buffer
     * @tparam FilterSize the number of cascading filters
     */
    template<typename FloatType, size_t FilterSize>
    class IIR {
    public:
        IIR() = default;

        void reset() {
            if (toReset.exchange(false)) {
                for (size_t i = 0; i < currentFilterNum; ++i) {
                    filters[i].reset();
                }
                for (size_t i = 0; i < currentFilterNum; ++i) {
                    svfFilters[i].reset();
                }
                bypassNextBlock = true;
            }
        }

        void setToRest() { toReset.store(true); }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            processSpec = spec;
            numChannels.store(spec.numChannels);
            sampleRate.store(static_cast<float>(spec.sampleRate));
            for (auto &f: filters) {
                f.prepare(spec);
            }
            for (auto &f: svfFilters) {
                f.prepare(spec);
            }
            setOrder(order.load());
            parallelBuffer.setSize(static_cast<int>(spec.numChannels),
                                   static_cast<int>(spec.maximumBlockSize));
        }

        /**
         * prepare for processing the incoming audio buffer
         * call it when you want to update filter parameters
         * @param buffer
         */
        void processPre(juce::AudioBuffer<FloatType> &buffer) {
            if (currentFilterStructure != filterStructure.load() || currentFilterType != filterType.load()) {
                currentFilterStructure = filterStructure.load();
                currentFilterType = filterType.load();
                shouldBeParallel = (currentFilterType == FilterType::peak) || (
                                       currentFilterType == FilterType::lowShelf) || (
                                       currentFilterType == FilterType::highShelf) || (
                                       currentFilterType == FilterType::bandShelf);
                shouldNotBeParallel = !shouldBeParallel;
                shouldBeParallel = shouldBeParallel && (currentFilterStructure == FilterStructure::parallel);
                shouldNotBeParallel = shouldNotBeParallel && (currentFilterStructure == FilterStructure::parallel);
                toReset.store(true);
                toUpdatePara.store(true);
            }
            if (shouldBeParallel) {
                parallelBuffer.makeCopyOf(buffer);
            }
            reset();
            if (toUpdatePara.exchange(false)) {
                updateCoeffs();
            }
        }

        /**
         * process the incoming audio buffer
         * for parallel filter, call it with the internal parallel buffer
         * @param buffer
         */
        template <bool isBypassed=false>
        void process(juce::AudioBuffer<FloatType> &buffer) {
            switch (currentFilterStructure) {
                case FilterStructure::iir: {
                    auto block = juce::dsp::AudioBlock<FloatType>(buffer);
                    auto context = juce::dsp::ProcessContextReplacing<FloatType>(block);
                    context.isBypassed = isBypassed || bypassNextBlock;
                    bypassNextBlock = false;
                    for (size_t i = 0; i < currentFilterNum; ++i) {
                        filters[i].process(context);
                    }
                    break;
                }
                case FilterStructure::svf: {
                    auto block = juce::dsp::AudioBlock<FloatType>(buffer);
                    auto context = juce::dsp::ProcessContextReplacing<FloatType>(block);
                    context.isBypassed = isBypassed || bypassNextBlock;
                    bypassNextBlock = false;
                    for (size_t i = 0; i < currentFilterNum; ++i) {
                        svfFilters[i].process(context);
                    }
                    break;
                }
                case FilterStructure::parallel: {
                    if (shouldBeParallel) {
                        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
                        auto context = juce::dsp::ProcessContextReplacing<FloatType>(block);
                        context.isBypassed = isBypassed || bypassNextBlock;
                        bypassNextBlock = false;
                        for (size_t i = 0; i < currentFilterNum; ++i) {
                            filters[i].process(context);
                        }
                        buffer.applyGain(parallelMultiplier);
                        break;
                    } else {
                        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
                        auto context = juce::dsp::ProcessContextReplacing<FloatType>(block);
                        context.isBypassed = isBypassed || bypassNextBlock;
                        bypassNextBlock = false;
                        for (size_t i = 0; i < currentFilterNum; ++i) {
                            filters[i].process(context);
                        }
                        break;
                    }
                }
            }
        }

        /**
         * add the processed parallel buffer to the incoming audio buffer
         * @param buffer
         */
        template <bool isBypassed=false>
        void processParallelPost(juce::AudioBuffer<FloatType> &buffer) {
            if (isBypassed) return;
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                auto *dest = buffer.getWritePointer(channel);
                auto *source = parallelBuffer.getWritePointer(channel);
                for (size_t idx = 0; idx < static_cast<size_t>(buffer.getNumSamples()); ++idx) {
                    dest[idx] = dest[idx] + source[idx];
                }
            }
        }

        /**
         * set the frequency of the filter
         * if frequency changes >= 2 octaves, the filter will reset
         * @param x frequency
         */
        template <bool update=true>
        void setFreq(const FloatType x) {
            const auto diff = std::max(static_cast<double>(x), freq.load()) /
                              std::min(static_cast<double>(x), freq.load());
            if (std::log10(diff) >= 2 && update && !useSVF.load()) {
                toReset.store(true);
            }
            freq.store(static_cast<double>(x));
            if (update) { toUpdatePara.store(true); }
        }

        FloatType getFreq() const { return static_cast<FloatType>(freq.load()); }

        /**
         * set the gain of the filter
         * @param x gain
         */
        template <bool update=true>
        void setGain(const FloatType x) {
            gain.store(static_cast<double>(x));
            switch (filterType.load()) {
                case peak:
                case bandShelf:
                case lowShelf:
                case highShelf:
                case tiltShelf: {
                    if (update) { toUpdatePara.store(true); }
                    break;
                }
                case lowPass:
                case highPass:
                case notch:
                case bandPass: {
                    break;
                }
            }
        }

        FloatType getGain() const { return static_cast<FloatType>(gain.load()); }

        /**
         * set gain and update coeffs immediately
         * @param x gain
         */
        void setGainNow(FloatType x) {
            gain.store(static_cast<double>(x));
            switch (currentFilterStructure) {
                case FilterStructure::iir:
                case FilterStructure::svf: {
                    updateCoeffs();
                    break;
                }
                case FilterStructure::parallel: {
                    if (shouldBeParallel) {
                        updateParallelGain(x);
                    } else {
                        updateCoeffs();
                    }
                }
            }
        }

        /**
         * set the Q value of the filter
         * @param x Q value
         */
        template <bool update=true>
        void setQ(const FloatType x) {
            q.store(static_cast<double>(x));
            if (update) { toUpdatePara.store(true); }
        }

        inline FloatType getQ() const { return static_cast<FloatType>(q.load()); }

        /**
         * set gain & Q and update coeffs immediately
         * @param g1 gain
         * @param q1 Q value
         */
        void setGainAndQNow(FloatType g1, FloatType q1) {
            gain.store(static_cast<double>(g1));
            q.store(static_cast<double>(q1));
            updateCoeffs();
        }

        /**
         * set the type of the filter, the filter will always reset
         * @param x filter type
         */
        template <bool update=true>
        void setFilterType(const FilterType x) {
            toReset.store(true);
            filterType.store(x);
            if (update) { toUpdatePara.store(true); }
        }

        inline FilterType getFilterType() const { return filterType.load(); }

        /**
         * set the order of the filter, the filter will always reset
         * @param x filter order
         */
        template <bool update=true>
        void setOrder(const size_t x) {
            order.store(x);
            if (update) {
                toReset.store(true);
                toUpdatePara.store(true);
            }
        }

        inline size_t getOrder() const { return order.load(); }

        /**
         * update filter coefficients
         * DO NOT call it unless you are sure what you are doing
         */
        void updateCoeffs() {
            if (!shouldBeParallel) {
                currentFilterNum = updateIIRCoeffs(currentFilterType, order.load(),
                                                   freq.load(), processSpec.sampleRate,
                                                   gain.load(), q.load(), coeffs);
            } else {
                if (currentFilterType == FilterType::peak) {
                    currentFilterNum = updateIIRCoeffs(FilterType::bandPass,
                                                       std::min(static_cast<size_t>(4), order.load()),
                                                       freq.load(), processSpec.sampleRate,
                                                       gain.load(), q.load(), coeffs);
                } else if (currentFilterType == FilterType::lowShelf) {
                    currentFilterNum = updateIIRCoeffs(FilterType::lowPass,
                                                       std::min(static_cast<size_t>(2), order.load()),
                                                       freq.load(), processSpec.sampleRate,
                                                       gain.load(), q.load(), coeffs);
                } else if (currentFilterType == FilterType::highShelf) {
                    currentFilterNum = updateIIRCoeffs(FilterType::highPass,
                                                       std::min(static_cast<size_t>(2), order.load()),
                                                       freq.load(), processSpec.sampleRate,
                                                       gain.load(), q.load(), coeffs);
                }

                updateParallelGain(gain.load());
            }
            switch (currentFilterStructure) {
                case FilterStructure::iir:
                case FilterStructure::parallel: {
                    for (size_t i = 0; i < currentFilterNum; i++) {
                        filters[i].updateFromBiquad(coeffs[i]);
                    }
                    break;
                }
                case FilterStructure::svf: {
                    for (size_t i = 0; i < currentFilterNum; i++) {
                        svfFilters[i].updateFromBiquad(coeffs[i]);
                    }
                }
            }
        }

        /**
         * get the num of channels
         * @return
         */
        inline juce::uint32 getNumChannels() const { return numChannels.load(); }

        /**
         * get the array of 2nd order filters
         * @return
         */
        std::array<IIRBase<FloatType>, FilterSize> &getFilters() { return filters; }

        void setFilterStructure(const FilterStructure x) {
            filterStructure.store(x);
        }

        bool getShouldBeParallel() const { return shouldBeParallel; }

        bool getShouldNotBeParallel() const { return shouldNotBeParallel; }

        juce::AudioBuffer<FloatType> &getParallelBuffer() { return parallelBuffer; }

    private:
        std::array<IIRBase<FloatType>, FilterSize> filters{};
        juce::AudioBuffer<FloatType> parallelBuffer;

        size_t currentFilterNum{1};
        std::atomic<double> freq = 1000, gain = 0, q = 0.707;
        std::atomic<size_t> order{2};
        std::atomic<FilterType> filterType{FilterType::peak};
        FilterType currentFilterType{FilterType::peak};
        bool bypassNextBlock{false};

        juce::dsp::ProcessSpec processSpec{48000, 512, 2};
        std::atomic<float> sampleRate{48000};
        std::atomic<juce::uint32> numChannels;

        std::atomic<bool> toUpdatePara = false, toReset = false;

        std::array<std::array<double, 6>, FilterSize> coeffs{};

        std::atomic<bool> useSVF{false};
        bool currentUseSVF{false};
        std::array<SVFBase<FloatType>, FilterSize> svfFilters{};

        std::atomic<FilterStructure> filterStructure{FilterStructure::iir};
        FilterStructure currentFilterStructure{FilterStructure::iir};
        bool shouldBeParallel{false}, shouldNotBeParallel{false};
        FloatType parallelMultiplier;

        static size_t updateIIRCoeffs(const FilterType filterType, const size_t n,
                                      const double f, const double fs, const double g0, const double q0,
                                      std::array<std::array<double, 6>, FilterSize> &coeffs) {
            return FilterDesign::updateCoeffs<FilterSize,
                MartinCoeff::get1LowShelf, MartinCoeff::get1HighShelf, MartinCoeff::get1TiltShelf,
                MartinCoeff::get1LowPass, MartinCoeff::get1HighPass,
                MartinCoeff::get2Peak,
                MartinCoeff::get2LowShelf, MartinCoeff::get2HighShelf, MartinCoeff::get2TiltShelf,
                MartinCoeff::get2LowPass, MartinCoeff::get2HighPass,
                MartinCoeff::get2BandPass, MartinCoeff::get2Notch>(
                filterType, n, f, fs, g0, q0, coeffs);
        }

        void updateParallelGain(double x) {
            parallelMultiplier = juce::Decibels::decibelsToGain<FloatType>(static_cast<FloatType>(x)) - FloatType(1);
        }
    };
}

#endif //ZLEQUALIZER_SINGLE_FILTER_HPP
