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
    void SingleFilter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        processSpec = spec;
    }

    template<typename FloatType>
    void SingleFilter<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<FloatType>(block);
        for (auto &f: filters) {
            f->process(context);
        }
    }

    template<typename FloatType>
    void SingleFilter<FloatType>::setFreq(FloatType x) {
        freq.store(static_cast<double>(x));
        updateParas();
    }

    template<typename FloatType>
    void SingleFilter<FloatType>::setGain(FloatType x) {
        gain.store(static_cast<double>(x));
        updateParas();
    }

    template<typename FloatType>
    void SingleFilter<FloatType>::setQ(FloatType x) {
        q.store(static_cast<double>(x));
        updateParas();
    }

    template<typename FloatType>
    void SingleFilter<FloatType>::setFilterType(zlIIR::FilterType x) {
        filterType.store(x);
        updateParas();
    }

    template<typename FloatType>
    void SingleFilter<FloatType>::setOrder(size_t x) {
        if (x != order.load()) {
            order.store(x);
            auto num = x > 1 ? x / 2 : 1;
            filters.clear();
            filters.reserve(num);
            for (size_t i = 0; i < num; i++) {
                filters[i] = std::make_unique<juce::dsp::IIR::Filter<FloatType>>();
            }
            updateParas();
        }
    }

    template<typename FloatType>
    void SingleFilter<FloatType>::updateParas() {
        auto coeff = DesignFilter::getCoeff(filterType.load(), freq.load(), processSpec.sampleRate,
                                            gain.load(), q.load(), order.load());
        for (size_t i = 0; i < coeff.size(); i++) {
            auto [a, b] = coeff[i];
            auto iirCoeff = juce::dsp::IIR::Coefficients<FloatType>(static_cast<FloatType>(b[0]),
                                                                    static_cast<FloatType>(b[1]),
                                                                    static_cast<FloatType>(b[2]),
                                                                    static_cast<FloatType>(a[0]),
                                                                    static_cast<FloatType>(a[1]),
                                                                    static_cast<FloatType>(a[2]));
            filters[i]->coefficients = iirCoeff;
        }
    }

    template
    class SingleFilter<float>;

    template
    class SingleFilter<double>;
}