// Copyright (C) 2025 - zsliu98
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
#include "../../chore/chore.hpp"
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

            }
        }

        void setToRest() { toReset.store(true); }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            processSpec = spec;
            numChannels.store(spec.numChannels);
            for (auto &f: filters) {
                f.prepare(spec);
            }
            for (auto &f: svfFilters) {
                f.prepare(spec);
            }
            setOrder(order.load());
            parallelBuffer.setSize(static_cast<int>(spec.numChannels),
                                   static_cast<int>(spec.maximumBlockSize));
            currentFreq.prepare(spec.sampleRate, 0.1);
            currentGain.prepare(spec.sampleRate, 0.001);
            currentQ.prepare(spec.sampleRate, 0.001);
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
                updateCoeffs();
            }
            if (shouldBeParallel) {
                parallelBuffer.makeCopyOf(buffer);
            }
            reset();
            if (toUpdatePara.exchange(false)) {
                updateCoeffs();
            }
            if (toUpdateFGQ.exchange(false)) {
                currentFreq.setTarget(freq.load());
                currentGain.setTarget(gain.load());
                currentQ.setTarget(q.load());
            }
        }

        /**
         * process the incoming audio buffer
         * for parallel filter, call it with the internal parallel buffer
         * @param buffer
         */
        template<bool isBypassed = false>
        void process(juce::AudioBuffer<FloatType> &buffer) {
            const auto f1 = currentFreq.isSmoothing();
            const auto f2 = currentGain.isSmoothing();
            const auto f3 = currentQ.isSmoothing();
            switch (currentFilterStructure) {
                case FilterStructure::iir: {
                    if (f1 || f2 || f3) {
                        processIIR<isBypassed, true>(buffer);
                    } else {
                        processIIR<isBypassed, false>(buffer);
                    }
                    break;
                }
                case FilterStructure::svf: {
                    if (f1 || f2 || f3) {
                        processSVF<isBypassed, true>(buffer);
                    } else {
                        processSVF<isBypassed, false>(buffer);
                    }
                    break;
                }
                case FilterStructure::parallel: {
                    if (shouldBeParallel) {
                        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
                        auto context = juce::dsp::ProcessContextReplacing<FloatType>(block);
                        context.isBypassed = isBypassed;
                        if (f2) {
                            const auto multiplier0 = parallelMultiplier;
                            if (f1 || f3) {
                                processIIR<isBypassed, true>(buffer);
                            } else {
                                processIIR<isBypassed, false>(buffer);
                                skipSmooth();
                            }
                            const auto multiplier2 = parallelMultiplier;
                            const auto w = static_cast<FloatType>(buffer.getNumSamples() - 1) / static_cast<FloatType>(buffer.getNumSamples());
                            const auto multiplier1 = multiplier0 * w + multiplier2 * (FloatType(1) - w);
                            buffer.applyGainRamp(0, buffer.getNumSamples(), multiplier1, multiplier2);
                        } else {
                            if (f1 || f3) {
                                processIIR<isBypassed, true>(buffer);
                            } else {
                                processIIR<isBypassed, false>(buffer);
                            }
                            buffer.applyGain(parallelMultiplier);
                        }
                        break;
                    } else {
                        if (f1 || f2 || f3) {
                            processIIR<isBypassed, true>(buffer);
                        } else {
                            processIIR<isBypassed, false>(buffer);
                        }
                        break;
                    }
                }
            }
        }

        template<bool isBypassed = false, bool isSmooth = false>
        void processIIR(juce::AudioBuffer<FloatType> &buffer) {
            const auto writerPointer = buffer.getArrayOfWritePointers();
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                if (isSmooth) updateCoeffs();
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                    auto sample = *(writerPointer[channel] + i);
                    for (size_t filterIdx = 0; filterIdx < currentFilterNum; ++filterIdx) {
                        sample = filters[filterIdx].processSample(static_cast<size_t>(channel), sample);
                    }
                    if (!isBypassed) {
                        *(writerPointer[channel] + i) = sample;
                    }
                }
            }
        }

        template<bool isBypassed = false, bool isSmooth = false>
        void processSVF(juce::AudioBuffer<FloatType> &buffer) {
            const auto writerPointer = buffer.getArrayOfWritePointers();
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                if (isSmooth) updateCoeffs();
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                    auto sample = *(writerPointer[static_cast<size_t>(channel)] + i);
                    for (size_t filterIdx = 0; filterIdx < currentFilterNum; ++filterIdx) {
                        sample = svfFilters[filterIdx].processSample(static_cast<size_t>(channel), sample);
                    }
                    if (!isBypassed) {
                        *(writerPointer[static_cast<size_t>(channel)] + i) = sample;
                    }
                }
            }
        }

        /**
         * add the processed parallel buffer to the incoming audio buffer
         * @param buffer
         */
        template<bool isBypassed = false>
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
         * @param x frequency
         */
        template<bool update = true, bool async = true, bool force = false>
        void setFreq(const FloatType x) {
            if (async) {
                freq.store(static_cast<double>(x));
                if (update) { toUpdateFGQ.store(true); }
            } else {
                if (force) {
                    currentFreq.setCurrentAndTarget(static_cast<double>(x));
                } else {
                    currentFreq.setTarget(static_cast<double>(x));
                }
            }
        }

        template<bool async = true>
        FloatType getFreq() const {
            if (async) {
                return static_cast<FloatType>(freq.load());
            } else {
                return static_cast<FloatType>(currentFreq.getCurrent());
            }
        }

        /**
         * set the gain of the filter
         * @param x gain
         */
        template<bool update = true, bool async = true, bool force = false>
        void setGain(const FloatType x) {
            if (async) {
                gain.store(static_cast<double>(x));
                if (update) toUpdateFGQ.store(true);
            } else {
                if (force) {
                    currentGain.setCurrentAndTarget(static_cast<double>(x));
                } else {
                    currentGain.setTarget(static_cast<double>(x));
                }
            }
        }

        template<bool async = true>
        FloatType getGain() const {
            if (async) {
                return static_cast<FloatType>(gain.load());
            } else {
                return static_cast<FloatType>(currentGain.getCurrent());
            }
        }

        /**
         * set the Q value of the filter
         * @param x Q value
         */
        template<bool update = true, bool async = true, bool force = false>
        void setQ(const FloatType x) {
            if (async) {
                q.store(static_cast<double>(x));
                if (update) toUpdateFGQ.store(true);
            } else {
                if (force) {
                    currentQ.setCurrentAndTarget(static_cast<double>(x));
                } else {
                    currentQ.setTarget(static_cast<double>(x));
                }
            }
        }

        template<bool async = true>
        FloatType getQ() const {
            if (async) {
                return static_cast<FloatType>(q.load());
            } else {
                return static_cast<FloatType>(currentQ.getCurrent());
            }
        }

        void skipSmooth() {
            currentFreq.setCurrentAndTarget(currentFreq.getTarget());
            currentGain.setCurrentAndTarget(currentGain.getTarget());
            currentQ.setCurrentAndTarget(currentQ.getTarget());
            updateCoeffs();
        }

        /**
         * set the type of the filter, the filter will always reset
         * @param x filter type
         */
        template<bool update = true>
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
        template<bool update = true>
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
            const auto nextFreq = currentFreq.getNext();
            const auto nextGain = currentGain.getNext();
            const auto nextQ = currentQ.getNext();
            if (!shouldBeParallel) {
                currentFilterNum = updateIIRCoeffs(currentFilterType, order.load(),
                                                   nextFreq, processSpec.sampleRate,
                                                   nextGain, nextQ, coeffs);
            } else {
                if (currentFilterType == FilterType::peak) {
                    currentFilterNum = updateIIRCoeffs(FilterType::bandPass,
                                                       std::min(static_cast<size_t>(4), order.load()),
                                                       nextFreq, processSpec.sampleRate,
                                                       nextGain, nextQ, coeffs);
                } else if (currentFilterType == FilterType::lowShelf) {
                    currentFilterNum = updateIIRCoeffs(FilterType::lowPass,
                                                       std::min(static_cast<size_t>(2), order.load()),
                                                       nextFreq, processSpec.sampleRate,
                                                       nextGain, nextQ, coeffs);
                } else if (currentFilterType == FilterType::highShelf) {
                    currentFilterNum = updateIIRCoeffs(FilterType::highPass,
                                                       std::min(static_cast<size_t>(2), order.load()),
                                                       nextFreq, processSpec.sampleRate,
                                                       nextGain, nextQ, coeffs);
                }
                updateParallelGain(nextGain);
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
        std::atomic<double> freq{1000.0}, gain{0.0}, q{0.707};
        zlChore::SmoothedValue<double, zlChore::Lin> currentGain{0.0};
        zlChore::SmoothedValue<double, zlChore::Mul> currentQ{0.707};
        zlChore::SmoothedValue<double, zlChore::FixMul> currentFreq{1000.0};
        std::atomic<size_t> order{2};
        std::atomic<FilterType> filterType{FilterType::peak};
        FilterType currentFilterType{FilterType::peak};

        juce::dsp::ProcessSpec processSpec{48000, 512, 2};
        std::atomic<juce::uint32> numChannels{2};

        std::atomic<bool> toUpdatePara{true}, toReset{true};
        std::atomic<bool> toUpdateFGQ{false};

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
