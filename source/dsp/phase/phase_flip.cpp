// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "phase_flip.hpp"

#include "../vector/vector.hpp"

namespace zlPhase {
    template<typename FloatType>
    void PhaseFlip<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        if (isON.load()) {
            const auto numSamples = static_cast<size_t>(buffer.getNumSamples());
            const auto numChannels = buffer.getNumChannels();
            for (int chan = 0; chan < numChannels; ++chan) {
                zlVector::multiply(buffer.getWritePointer(chan), FloatType(-1.f), numSamples);
            }
        }
    }

    template<typename FloatType>
    void PhaseFlip<FloatType>::process(juce::dsp::AudioBlock<FloatType> block) {
        if (isON.load()) {
            const auto numSamples = block.getNumSamples();
            const auto numChannels = block.getNumChannels();
            for (size_t chan = 0; chan < numChannels; ++chan) {
                zlVector::multiply(block.getChannelPointer(chan), FloatType(-1.f), numSamples);
            }
        }
    }

    template
    class PhaseFlip<float>;

    template
    class PhaseFlip<double>;
} // zlPhase
