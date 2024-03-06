// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "sample_delay.hpp"

namespace zlDelay {
    template<typename FloatType>
    void SampleDelay<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        delayDSP.prepare(spec);
    }

    template<typename FloatType>
    void SampleDelay<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        process(block);
    }

    template<typename FloatType>
    void SampleDelay<FloatType>::process(juce::dsp::AudioBlock<FloatType> block) {
        if (delaySamples.load() == 0) { return; }
        if (static_cast<int>(delayDSP.getDelay()) != delaySamples.load()) {
            delayDSP.setDelay(static_cast<FloatType>(delaySamples.load()));
        }
        juce::dsp::ProcessContextReplacing<FloatType> context(block);
        delayDSP.process(context);
    }

    template
    class SampleDelay<float>;

    template
    class SampleDelay<double>;
} // zlDelay
