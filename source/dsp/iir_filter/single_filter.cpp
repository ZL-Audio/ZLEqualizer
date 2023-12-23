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
        setOrder(order.load());
        updateParas();
    }

    template<typename FloatType>
    void SingleFilter<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<FloatType>(block);
        for (auto &f: filters) {
            f.process(context);
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
//        logger.logMessage("set order to " + juce::String(x));
        order.store(x);
        updateParas();
    }

    template<typename FloatType>
    void SingleFilter<FloatType>::updateParas() {
//        logger.logMessage("______________________");
//        logger.logMessage("order is: " + juce::String(order.load()));
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
//        logger.logMessage("coeff size is: " + juce::String(coeff.size()));
        for (size_t i = 0; i < coeff.size(); i++) {
            auto [a, b] = coeff[i];
            *filters[i].state = std::array<FloatType, 6>{static_cast<FloatType>(b[0]),
                                                         static_cast<FloatType>(b[1]),
                                                         static_cast<FloatType>(b[2]),
                                                         static_cast<FloatType>(a[0]),
                                                         static_cast<FloatType>(a[1]),
                                                         static_cast<FloatType>(a[2])};

//            logger.logMessage("coeff are: " + juce::String(b[0]) + " " + juce::String(b[1]) + " " + juce::String(b[2]));
//            logger.logMessage("coeff are: " + juce::String(a[0]) + " " + juce::String(a[1]) + " " + juce::String(a[2]));
        }
//        logger.logMessage("______________________");
    }

    template
    class SingleFilter<float>;

    template
    class SingleFilter<double>;
}