// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_FORWARD_COMPRESSOR_HPP
#define ZLEQUALIZER_FORWARD_COMPRESSOR_HPP

#include <juce_dsp/juce_dsp.h>

#include "computer/computer.hpp"
#include "detector/detector.hpp"
#include "tracker/tracker.hpp"

namespace zlCompressor {
    /**
     * a forward compressor
     * @tparam FloatType
     */
    template<typename FloatType>
    class ForwardCompressor {
    public:
        ForwardCompressor() = default;

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * process the audio buffer and return the compression gain (in gain)
         * @param buffer side chain audio buffer
         * @return gain (in gain)
         */
        FloatType process(juce::AudioBuffer<FloatType> buffer);

        inline KneeComputer<FloatType> &getComputer() { return computer; }

        inline Detector<FloatType> &getDetector() { return detector; }

        inline RMSTracker<FloatType> &getTracker() { return tracker; }

        inline void setBaseLine(const FloatType x) { baseLine = x; }

        inline FloatType getBaseLine() const { return baseLine; }

    private:
        KneeComputer<FloatType> computer;
        Detector<FloatType> detector;
        RMSTracker<FloatType> tracker;
        FloatType baseLine{0};
    };
}

#endif //ZLEQUALIZER_FORWARD_COMPRESSOR_HPP
