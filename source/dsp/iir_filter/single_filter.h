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

#ifndef ZLEQUALIZER_SINGLE_FILTER_H
#define ZLEQUALIZER_SINGLE_FILTER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "coeff/design_filter.h"

namespace zlIIR {
    template<typename FloatType>
    class Filter {
    public:
        Filter() = default;

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<FloatType> &buffer);

        void setFreq(FloatType x);

        inline FloatType getFreq() { return static_cast<FloatType>(freq.load()); }

        void setGain(FloatType x);

        inline FloatType getGain() { return static_cast<FloatType>(gain.load()); }

        void setQ(FloatType x);

        inline FloatType getQ() { return static_cast<FloatType>(q.load()); }

        void setFilterType(FilterType x);

        inline FilterType getFilterType() { return filterType.load(); }

        void setOrder(size_t x);

        inline size_t getOrder() { return order.load(); }

    private:
        std::vector<juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<FloatType>, juce::dsp::IIR::Coefficients<FloatType>>> filters;
        std::atomic<double> freq = 1000, gain = 0, q = 0.707;
        std::atomic<size_t> order = 2;
        std::atomic<FilterType> filterType = FilterType::peak;
        juce::dsp::ProcessSpec processSpec{48000, 512, 2};

        void updateParas();
    };
}

#endif //ZLEQUALIZER_SINGLE_FILTER_H
