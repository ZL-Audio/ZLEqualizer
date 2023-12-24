// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_IIR_FILTER_H
#define ZLEQUALIZER_IIR_FILTER_H

#include "single_filter.h"

namespace zlIIR {
    template<typename FloatType>
    class DynamicFilter {
    public:
        DynamicFilter() = default;

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<FloatType> &buffer);

        inline void setFreq(FloatType x) {
            mFilter.setFreq(x);
            tFilter.setFreq(x);
        }

        inline void setGainMain(FloatType x) {
            mFilter.setGain(x);
        }

        inline void setGainTarget(FloatType x) {
            tFilter.setGain(x);
        }

        inline void setQMain(FloatType x) {
            mFilter.setQ(x);
        }

        inline void setQTarget(FloatType x) {
            tFilter.setQ(x);
        }

        inline void setFilterType(FilterType x) {
            mFilter.setFilterType(x);
            tFilter.setFilterType(x);
        }

        inline void setOrder(size_t x) {
            mFilter.setOrder(x);
            tFilter.setOrder(x);
        }

        inline void setTargetProportion(FloatType x) {
            mixer.setWetMixProportion(x);
        }

        inline void setDynamicON(bool x) {
            dynamicON.store(x);
        }

    private:
        SingleFilter<FloatType> mFilter, tFilter;
        juce::dsp::DryWetMixer<FloatType> mixer;
        juce::AudioBuffer<FloatType> tBuffer;
        std::atomic<bool> dynamicON = false;
    };
}


#endif //ZLEQUALIZER_IIR_FILTER_H
