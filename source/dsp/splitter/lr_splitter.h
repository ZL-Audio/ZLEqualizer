// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_LRSPLITER_H
#define ZLEQUALIZER_LRSPLITER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlSplitter {
    template<typename FloatType>
    class LRSplitter {
    public:
        LRSplitter() = default;

        void prepare(const juce::dsp::ProcessSpec &spec);

        void split(juce::AudioBuffer<FloatType> &buffer);

        void combine(juce::AudioBuffer<FloatType> &buffer);

        inline juce::AudioBuffer<FloatType> &getLBuffer() { return lBuffer; }

        inline juce::AudioBuffer<FloatType> &getRBuffer() { return rBuffer; }

    private:
        juce::AudioBuffer<FloatType> lBuffer, rBuffer;
    };
}

#endif //ZLEQUALIZER_LRSPLITER_H
