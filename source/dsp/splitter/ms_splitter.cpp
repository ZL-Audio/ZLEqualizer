// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ms_splitter.hpp"
namespace zlSplitter {
    template<typename FloatType>
    void MSSplitter<FloatType>::reset() {
        mBuffer.clear();
        sBuffer.clear();
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        mBuffer.setSize(1, static_cast<int>(spec.maximumBlockSize));
        sBuffer.setSize(1, static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::split(juce::AudioBuffer<FloatType> &buffer) {
        auto lBuffer = buffer.getReadPointer(0);
        auto rBuffer = buffer.getReadPointer(1);
        auto _mBuffer = mBuffer.getWritePointer(0);
        auto _sBuffer = sBuffer.getWritePointer(0);
        for (size_t i = 0; i < static_cast<size_t>(buffer.getNumSamples()); ++i) {
            _mBuffer[i] = FloatType(0.5) * (lBuffer[i] + rBuffer[i]);
            _sBuffer[i] = FloatType(0.5) * (lBuffer[i] - rBuffer[i]);
        }
        mBuffer.setSize(1, buffer.getNumSamples(), true, false, true);
        sBuffer.setSize(1, buffer.getNumSamples(), true, false, true);
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::combine(juce::AudioBuffer<FloatType> &buffer) {
        auto lBuffer = buffer.getWritePointer(0);
        auto rBuffer = buffer.getWritePointer(1);
        auto _mBuffer = mBuffer.getReadPointer(0);
        auto _sBuffer = sBuffer.getReadPointer(0);
        for (size_t i = 0; i < static_cast<size_t>(buffer.getNumSamples()); ++i) {
            lBuffer[i] = _mBuffer[i] + _sBuffer[i];
            rBuffer[i] = _mBuffer[i] - _sBuffer[i];
        }
    }

    template
    class MSSplitter<float>;

    template
    class MSSplitter<double>;
}