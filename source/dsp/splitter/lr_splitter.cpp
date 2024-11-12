// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "lr_splitter.hpp"

namespace zlSplitter {
    template<typename FloatType>
    void LRSplitter<FloatType>::reset() {
        lBuffer.clear();
        rBuffer.clear();
    }

    template<typename FloatType>
    void LRSplitter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        lBuffer.setSize(1, static_cast<int>(spec.maximumBlockSize));
        rBuffer.setSize(1, static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    void LRSplitter<FloatType>::split(juce::AudioBuffer<FloatType> &buffer) {
        lBuffer.setSize(1, buffer.getNumSamples(), true, false, true);
        rBuffer.setSize(1, buffer.getNumSamples(), true, false, true);
        lBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
        rBuffer.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    }

    template<typename FloatType>
    void LRSplitter<FloatType>::combine(juce::AudioBuffer<FloatType> &buffer) {
        buffer.copyFrom(0, 0, lBuffer, 0, 0, buffer.getNumSamples());
        buffer.copyFrom(1, 0, rBuffer, 0, 0, buffer.getNumSamples());
    }

    template
    class LRSplitter<float>;

    template
    class LRSplitter<double>;
}