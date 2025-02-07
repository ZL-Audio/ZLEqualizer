// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "forward_compressor.hpp"

namespace zlCompressor {
    template<typename FloatType>
    void ForwardCompressor<FloatType>::reset() {
        detector.reset();
        tracker.reset();
    }

    template<typename FloatType>
    void ForwardCompressor<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        detector.prepare(spec);
        tracker.prepare(spec);
    }

    template<typename FloatType>
    FloatType ForwardCompressor<FloatType>::process(juce::AudioBuffer<FloatType> buffer) {
        tracker.process(buffer);
        auto x = tracker.getMomentaryLoudness() - baseLine.load();
        x = computer.process(x);
        x = juce::Decibels::decibelsToGain(x);
        detector.setBufferSize(buffer.getNumSamples());
        x = detector.process(x);
        return x;
    }

    template
    class ForwardCompressor<float>;

    template
    class ForwardCompressor<double>;
}