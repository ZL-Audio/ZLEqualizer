// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "dynamic_filter.h"

namespace zlIIR {
    template<typename FloatType>
    void DynamicFilter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        mFilter.prepare(spec);
        tFilter.prepare(spec);
        mixer.prepare(spec);
        tBuffer.setSize(static_cast<int>(spec.numChannels),
                        static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    void DynamicFilter<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        mFilter.process(buffer);
        if (dynamicON.load()) {
            tBuffer.makeCopyOf(buffer, true);
            tFilter.process(tBuffer);
            mixer.pushDrySamples(juce::dsp::AudioBlock<FloatType>(buffer));
            mixer.mixWetSamples(juce::dsp::AudioBlock<FloatType>(tBuffer));
        }
    }

    template
    class DynamicFilter<float>;

    template
    class DynamicFilter<double>;
}