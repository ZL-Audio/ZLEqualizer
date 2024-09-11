// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLPHASE_PHASE_FLIP_HPP
#define ZLPHASE_PHASE_FLIP_HPP

#include <juce_dsp/juce_dsp.h>

namespace zlPhase {
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

#endif //PHASE_FLIP_HPP
