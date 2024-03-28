// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef ZLECOMP_TRACKER_H
#define ZLECOMP_TRACKER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlCompressor {
    template<typename FloatType>
    class VirtualTracker {
    public:
        VirtualTracker() = default;

        virtual ~VirtualTracker() = default;

        virtual void reset() = 0;

        virtual void prepare(const juce::dsp::ProcessSpec &spec) = 0;

        virtual void process(const juce::AudioBuffer<FloatType> &buffer) = 0;

        virtual void setMomentarySize(size_t mSize) = 0;

        virtual void setMaximumMomentarySize(size_t mSize) = 0;

        virtual FloatType getMomentaryLoudness() = 0;
    };
}

#endif //ZLECOMP_TRACKER_H
