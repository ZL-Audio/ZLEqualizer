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

#include "virtual_tracker.h"

namespace zlCompressor {

    template<typename FloatType>
    class RMSTracker : VirtualTracker<FloatType> {
    public:
        RMSTracker() = default;

        ~RMSTracker() override;

        void prepare(const juce::dsp::ProcessSpec &spec) override;

        void reset() override;

        void setMomentarySize(size_t mSize) override;

        inline size_t getMomentarySize() {
            return loudnessBuffer.capacity();
        }

        inline FloatType getBufferPeak() override {
            return juce::Decibels::gainToDecibels(peak);
        }

        inline FloatType getMomentaryLoudness() override {
            FloatType meanSquare = 0;
            if (loudnessBuffer.size() > 0) {
                meanSquare = mLoudness / static_cast<FloatType>(loudnessBuffer.size());
            }
            return juce::Decibels::gainToDecibels(meanSquare) * static_cast<FloatType>(0.5);
        }

        inline FloatType getIntegratedLoudness() override {
            FloatType meanSquare = 0;
            if (numBuffer > 0) {
                meanSquare = iLoudness / static_cast<FloatType>(numBuffer);
            }
            return secondPerBuffer * juce::Decibels::gainToDecibels(meanSquare) *
                   static_cast<FloatType>(0.5);
        }

        inline FloatType getIntegratedTotalLoudness() override {
            return getIntegratedLoudness() * static_cast<FloatType>(numBuffer);
        }

        void process(const juce::AudioBuffer<FloatType> &buffer) override;

    private:
        size_t numBuffer = 0;
        FloatType peak = 0, mLoudness = 0, iLoudness = 0;
        FloatType secondPerBuffer = FloatType(0.01);
        boost::circular_buffer<FloatType> loudnessBuffer;
    };

} // zldetector

#endif //ZLECOMP_RMS_TRACKER_H
