// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "single_filter.hpp"

namespace zlFilter {
    template<typename FloatType>
    void IIR<FloatType>::reset() {
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

    template<typename FloatType>
    void IIR<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
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

    template<typename FloatType>
    void IIR<FloatType>::processPre(juce::AudioBuffer<FloatType> &buffer) {
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

    template<typename FloatType>
    void IIR<FloatType>::process(juce::AudioBuffer<FloatType> &buffer, bool isBypassed) {
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
                }
            }
        }
    }

    template<typename FloatType>
    void IIR<FloatType>::processParallelPost(juce::AudioBuffer<FloatType> &buffer, bool isBypassed) {
        if (shouldBeParallel) {
            if (isBypassed) return;
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                auto *dest = buffer.getWritePointer(channel);
                auto *source = parallelBuffer.getWritePointer(channel);
                for (size_t idx = 0; idx < static_cast<size_t>(buffer.getNumSamples()); ++idx) {
                    dest[idx] = dest[idx] + source[idx];
                }
            }
        } else {
            auto block = juce::dsp::AudioBlock<FloatType>(buffer);
            auto context = juce::dsp::ProcessContextReplacing<FloatType>(block);
            context.isBypassed = isBypassed;
            for (size_t i = 0; i < currentFilterNum; ++i) {
                filters[i].process(context);
            }
        }
    }


    template<typename FloatType>
    void IIR<FloatType>::setFreq(const FloatType x, const bool update) {
        const auto diff = std::max(static_cast<double>(x), freq.load()) /
                          std::min(static_cast<double>(x), freq.load());
        if (std::log10(diff) >= 2 && update && !useSVF.load()) {
            toReset.store(true);
        }
        freq.store(static_cast<double>(x));
        if (update) { toUpdatePara.store(true); }
    }

    template<typename FloatType>
    void IIR<FloatType>::setGain(const FloatType x, const bool update) {
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

    template<typename FloatType>
    void IIR<FloatType>::setGainNow(FloatType x) {
        gain.store(static_cast<double>(x));
        switch (currentFilterStructure) {
            case FilterStructure::iir:
            case FilterStructure::svf: {
                updateCoeffs();
                break;
            }
            case FilterStructure::parallel: {
                updateParallelGain(x);
            }
        }
    }

    template<typename FloatType>
    void IIR<FloatType>::setQ(const FloatType x, const bool update) {
        q.store(static_cast<double>(x));
        if (update) { toUpdatePara.store(true); }
    }

    template<typename FloatType>
    void IIR<FloatType>::setGainAndQNow(FloatType g1, FloatType q1) {
        bool shouldUpdateCoeffs{false};
        if (std::abs(static_cast<double>(q1) - q.load()) >= 0.00001) {
            q.store(static_cast<double>(q1));
            shouldUpdateCoeffs = true;
        }
        gain.store(static_cast<double>(g1));
        switch (currentFilterStructure) {
            case FilterStructure::iir:
            case FilterStructure::svf: {
                updateCoeffs();
                break;
            }
            case FilterStructure::parallel: {
                if (shouldUpdateCoeffs) {
                    updateCoeffs();
                } else {
                    updateParallelGain(g1);
                }
            }
        }
    }

    template<typename FloatType>
    void IIR<FloatType>::setFilterType(const FilterType x, const bool update) {
        toReset.store(true);
        filterType.store(x);
        if (update) { toUpdatePara.store(true); }
    }

    template<typename FloatType>
    void IIR<FloatType>::setOrder(const size_t x, const bool update) {
        order.store(x);
        if (update) {
            toReset.store(true);
            toUpdatePara.store(true);
        }
    }

    template<typename FloatType>
    void IIR<FloatType>::updateCoeffs() {
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

    template
    class IIR<float>;

    template
    class IIR<double>;
}
