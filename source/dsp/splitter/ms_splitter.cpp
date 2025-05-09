// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ms_splitter.hpp"

namespace zldsp::splitter {
    template<typename FloatType>
    void MSSplitter<FloatType>::reset() {
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        juce::ignoreUnused(spec);
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::split(juce::AudioBuffer<FloatType> &buffer) {
        auto l_vector = kfr::make_univector(buffer.getWritePointer(0),
            static_cast<size_t>(buffer.getNumSamples()));
        auto r_vector = kfr::make_univector(buffer.getWritePointer(1),
            static_cast<size_t>(buffer.getNumSamples()));

        l_vector = FloatType(0.5) * (l_vector + r_vector);
        r_vector = l_vector - r_vector;

        m_buffer_.setDataToReferTo(buffer.getArrayOfWritePointers(), 1, 0, buffer.getNumSamples());
        s_buffer_.setDataToReferTo(buffer.getArrayOfWritePointers() + 1, 1, 0, buffer.getNumSamples());
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::combine(juce::AudioBuffer<FloatType> &buffer) {
        auto m_vector = kfr::make_univector(buffer.getWritePointer(0),
            static_cast<size_t>(buffer.getNumSamples()));
        auto s_vector = kfr::make_univector(buffer.getWritePointer(1),
            static_cast<size_t>(buffer.getNumSamples()));
        m_vector = m_vector + s_vector;
        s_vector = m_vector - s_vector - s_vector;
    }

    template
    class MSSplitter<float>;

    template
    class MSSplitter<double>;
}
