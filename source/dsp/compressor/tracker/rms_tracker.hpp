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

#include "virtual_tracker.hpp"

namespace zlCompressor {
    /**
     * a tracker that tracks the momentary RMS loudness of the audio signal
     * @tparam FloatType
     */
    template<typename FloatType>
    class RMSTracker : VirtualTracker<FloatType> {
    public:
        RMSTracker() = default;

        ~RMSTracker() override;

        void reset() override;

        void prepare(const juce::dsp::ProcessSpec &spec) override;

        void process(const juce::AudioBuffer<FloatType> &buffer) override;

        void setMomentarySize(size_t mSize) override;

        inline size_t getMomentarySize() {
            return loudnessBuffer.capacity();
        }

        inline FloatType getMomentaryLoudness() override {
            FloatType meanSquare =
                    loudnessBuffer.size() > 0 ? mLoudness / static_cast<FloatType>(loudnessBuffer.size()) : 0;

            return juce::Decibels::gainToDecibels(meanSquare) * static_cast<FloatType>(0.5);
        }

    private:
        FloatType mLoudness = 0;
        FloatType secondPerBuffer = FloatType(0.01);
        boost::circular_buffer<FloatType> loudnessBuffer{1};
    };

} // zldetector

#endif //ZLECOMP_RMS_TRACKER_H
