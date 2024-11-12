// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLECOMP_RMS_TRACKER_H
#define ZLECOMP_RMS_TRACKER_H

#include <juce_dsp/juce_dsp.h>

#include "../../container/container.hpp"

namespace zlCompressor {
    /**
     * a tracker that tracks the momentary RMS loudness of the audio signal
     * @tparam FloatType
     */
    template<typename FloatType>
    class RMSTracker {
    public:
        inline static FloatType minusInfinityDB = -240;

        RMSTracker() = default;

        ~RMSTracker();

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(const juce::AudioBuffer<FloatType> &buffer);

        void setMomentarySeconds(FloatType x);

        void setMomentarySize(size_t mSize);

        void setMaximumMomentarySize(size_t mSize);

        inline size_t getMomentarySize() const {
            return currentSize.load();
        }

        FloatType getMomentaryLoudness();

    private:
        FloatType mLoudness {0};
        zlContainer::CircularBuffer<FloatType> loudnessBuffer{1};

        std::atomic<double> sampleRate{44100};
        std::atomic<FloatType> currentSeconds{0};
        std::atomic<size_t> currentSize{1};
    };
} // zldetector

#endif //ZLECOMP_RMS_TRACKER_H