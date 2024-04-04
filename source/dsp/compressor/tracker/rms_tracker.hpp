// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef ZLECOMP_RMS_TRACKER_H
#define ZLECOMP_RMS_TRACKER_H

#include <boost/circular_buffer.hpp>
#include <juce_dsp/juce_dsp.h>

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
        std::atomic<FloatType> mLoudness {0};
        boost::circular_buffer<FloatType> loudnessBuffer{1};

        std::atomic<double> sampleRate{44100};
        std::atomic<FloatType> currentSeconds{0};
        std::atomic<size_t> currentSize{1};
    };

} // zldetector

#endif //ZLECOMP_RMS_TRACKER_H