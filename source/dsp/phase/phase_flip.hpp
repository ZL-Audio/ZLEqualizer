// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

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
