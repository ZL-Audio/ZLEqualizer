// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_PHASE_PHASE_FLIP_HPP
#define ZL_PHASE_PHASE_FLIP_HPP

#include <juce_dsp/juce_dsp.h>

namespace zlPhase {
    /**
     * phase-flip the input audio buffer
     * @tparam FloatType the float type of input audio buffer
     */
    template<typename FloatType>
    class PhaseFlip {
    public:
        void process(juce::AudioBuffer<FloatType> &buffer);

        void process(juce::dsp::AudioBlock<FloatType> block);

        void setON(const bool f) { isON.store(f); }

    private:
        std::atomic<bool> isON;
    };
} // zlPhase

#endif //ZL_PHASE_PHASE_FLIP_HPP
