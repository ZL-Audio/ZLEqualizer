// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

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
        x = detector.process(x);
        return x;
    }

    template
    class ForwardCompressor<float>;

    template
    class ForwardCompressor<double>;
}