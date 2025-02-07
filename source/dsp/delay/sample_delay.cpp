// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "sample_delay.hpp"

namespace zlDelay {
    template<typename FloatType>
    void SampleDelay<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        delayDSP.prepare(spec);
        sampleRate.store(spec.sampleRate);
        delaySamples.store(static_cast<int>(static_cast<double>(delaySeconds.load()) * spec.sampleRate));
        toUpdateDelay.store(true);
    }

    template<typename FloatType>
    void SampleDelay<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        process(block);
    }

    template<typename FloatType>
    void SampleDelay<FloatType>::process(juce::dsp::AudioBlock<FloatType> block) {
        if (toUpdateDelay.exchange(false)) {
            currentDelaySamples = delaySamples.load();
            delayDSP.setDelay(static_cast<FloatType>(currentDelaySamples));
        }
        if (currentDelaySamples == 0) { return; }
        juce::dsp::ProcessContextReplacing<FloatType> context(block);
        delayDSP.process(context);
    }

    template
    class SampleDelay<float>;

    template
    class SampleDelay<double>;
} // zlDelay
