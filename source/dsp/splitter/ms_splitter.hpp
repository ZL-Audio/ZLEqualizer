// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_MS_SPLITTER_HPP
#define ZLEQUALIZER_MS_SPLITTER_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlSplitter {
    /**
     * a splitter that splits the stereo audio signal input mid signal and side signal
     * @tparam FloatType
     */
    template<typename FloatType>
    class MSSplitter {
    public:
        MSSplitter() = default;

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * split the audio buffer into internal mid buffer and side buffer
         * @param buffer
         */
        void split(juce::AudioBuffer <FloatType> &buffer);

        /**
         * combine the internal mid buffer and side buffer into the audio buffer
         * @param buffer
         */
        void combine(juce::AudioBuffer <FloatType> &buffer);

        inline juce::AudioBuffer<FloatType> &getMBuffer() { return mBuffer; }

        inline juce::AudioBuffer<FloatType> &getSBuffer() { return sBuffer; }

    private:
        juce::AudioBuffer <FloatType> mBuffer, sBuffer;
    };
}

#endif //ZLEQUALIZER_MS_SPLITTER_HPP
