// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

//
// Created by Zishu Liu on 12/19/23.
//

#include "single_filter.h"

namespace zlIIR {
    template<typename FloatType>
    void Filter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        processSpec = spec;
        setOrder(order.load());
        updateParas();
    }

    template<typename FloatType>
    void Filter<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<FloatType>(block);
        for (auto &f: filters) {
            f.process(context);
        }
    }

    template<typename FloatType>
    void Filter<FloatType>::setFreq(FloatType x) {
        freq.store(static_cast<double>(x));
        updateParas();
    }

    template<typename FloatType>
    void Filter<FloatType>::setGain(FloatType x) {
        gain.store(static_cast<double>(x));
        updateParas();
    }

    template<typename FloatType>
    void Filter<FloatType>::setQ(FloatType x) {
        q.store(static_cast<double>(x));
        updateParas();
    }

    template<typename FloatType>
    void Filter<FloatType>::setFilterType(zlIIR::FilterType x) {
        filterType.store(x);
        updateParas();
    }

    template<typename FloatType>
    void Filter<FloatType>::setOrder(size_t x) {
        order.store(x);
        updateParas();
    }

    template<typename FloatType>
    void Filter<FloatType>::updateParas() {
        auto coeff = DesignFilter::getCoeff(filterType.load(),
                                            freq.load(), processSpec.sampleRate,
                                            gain.load(), q.load(), order.load());
        if (coeff.size() != filters.size()) {
            filters.clear();
            filters.resize(coeff.size());
            for (size_t i = 0; i < coeff.size(); i++) {
                filters[i].prepare(processSpec);
            }
        }
        for (size_t i = 0; i < coeff.size(); i++) {
            auto [a, b] = coeff[i];
            std::array<FloatType, 6> finalCoeff{};
            for (size_t j = 0; j < 6; ++j) {
                finalCoeff[j] = static_cast<FloatType>(j < 3 ? b[j] : a[j - 3]);
            }
            *filters[i].state = finalCoeff;
        }
    }

    template
    class Filter<float>;

    template
    class Filter<double>;
}