// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "dynamic_iir_filter.h"

namespace zlDynamicFilter {
    template<typename FloatType>
    void DynamicIIRFilter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        mFilter.prepare(spec);
        tFilter.prepare(spec);
        sFilter.prepare(spec);
        compressor.prepare(spec);
        mixer.prepare(spec);
        tBuffer.setSize(static_cast<int>(spec.numChannels),
                        static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    void DynamicIIRFilter<FloatType>::process(juce::AudioBuffer<FloatType> &mBuffer, juce::AudioBuffer<FloatType> &sBuffer) {
        mFilter.process(mBuffer);
        if (dynamicON.load()) {
            sFilter.process(sBuffer);
            auto dryMixPortion = compressor.process(sBuffer);
            if (dryMixPortion < 1.0) {
                mixer.setWetMixProportion(1 - dryMixPortion);
                mixer.pushDrySamples(juce::dsp::AudioBlock<FloatType>(mBuffer));
                tBuffer.makeCopyOf(mBuffer, true);
                tFilter.process(tBuffer);
                mixer.mixWetSamples(juce::dsp::AudioBlock<FloatType>(tBuffer));
            }
        }
    }

    template
    class DynamicIIRFilter<float>;

    template
    class DynamicIIRFilter<double>;
}