// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ms_splitter.hpp"

namespace zlSplitter {
    template<typename FloatType>
    void MSSplitter<FloatType>::reset() {
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        juce::ignoreUnused(spec);
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::split(juce::AudioBuffer<FloatType> &buffer) {
        auto l_buffer = buffer.getWritePointer(0);
        auto r_buffer = buffer.getWritePointer(1);
        for (size_t i = 0; i < static_cast<size_t>(buffer.getNumSamples()); ++i) {
            const auto l = l_buffer[i], r = r_buffer[i];
            l_buffer[i] = FloatType(0.5) * (l + r);
            r_buffer[i] = FloatType(0.5) * (l - r);
        }

        mBuffer.setDataToReferTo(buffer.getArrayOfWritePointers(), 1, 0, buffer.getNumSamples());
        sBuffer.setDataToReferTo(buffer.getArrayOfWritePointers() + 1, 1, 0, buffer.getNumSamples());
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::combine(juce::AudioBuffer<FloatType> &buffer) {
        auto m_buffer = buffer.getWritePointer(0);
        auto s_buffer = buffer.getWritePointer(1);
        for (size_t i = 0; i < static_cast<size_t>(buffer.getNumSamples()); ++i) {
            const auto m = m_buffer[i], s = s_buffer[i];
            m_buffer[i] = m + s;
            s_buffer[i] = m - s;
        }
    }

    template
    class MSSplitter<float>;

    template
    class MSSplitter<double>;
}
