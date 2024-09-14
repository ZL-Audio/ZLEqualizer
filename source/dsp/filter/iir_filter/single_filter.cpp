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
            for (size_t i = 0; i < filterNum.load(); ++i) {
                filters[i].reset();
            }
            for (size_t i = 0; i < filterNum.load(); ++i) {
                svfFilters[i].reset();
            }
            bypassNextBlock.store(true);
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
    }

    template<typename FloatType>
    void IIR<FloatType>::process(juce::AudioBuffer<FloatType> &buffer, bool isBypassed) {
        if (currentUseSVF != useSVF.load()) {
            currentUseSVF = useSVF.load();
            toReset.store(true);
            toUpdatePara.store(true);
        }
        reset();
        updateParas();
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<FloatType>(block);
        context.isBypassed = isBypassed || bypassNextBlock.exchange(false);
        if (!currentUseSVF) {
            for (size_t i = 0; i < filterNum.load(); ++i) {
                filters[i].process(context);
            }
        } else {
            for (size_t i = 0; i < filterNum.load(); ++i) {
                svfFilters[i].process(context);
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
    void IIR<FloatType>::setQ(const FloatType x, const bool update) {
        q.store(static_cast<double>(x));
        if (update) { toUpdatePara.store(true); }
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
    bool IIR<FloatType>::updateParas() {
        if (toUpdatePara.exchange(false)) {
            filterNum.store(updateIIRCoeffs(filterType.load(), order.load(),
                                            freq.load(), processSpec.sampleRate,
                                            gain.load(), q.load(), coeffs));
            if (!currentUseSVF) {
                for (size_t i = 0; i < filterNum.load(); i++) {
                    filters[i].updateFromBiquad(coeffs[i]);
                }
            } else {
                for (size_t i = 0; i < filterNum.load(); i++) {
                    svfFilters[i].updateFromBiquad(coeffs[i]);
                }
            }
            return true;
        }
        return false;
    }

    template
    class IIR<float>;

    template
    class IIR<double>;
}
