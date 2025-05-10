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

#include "../vector/vector.hpp"

namespace zldsp::phase {
    /**
     * phase-flip the input audio buffer
     * @tparam FloatType the float type of input audio buffer
     */
    template<typename FloatType>
    class PhaseFlip {
    public:
        void process(juce::AudioBuffer<FloatType> &buffer) {
            if (is_on_.load()) {
                const auto num_samples = static_cast<size_t>(buffer.getNumSamples());
                const auto num_channels = buffer.getNumChannels();
                for (int chan = 0; chan < num_channels; ++chan) {
                    zldsp::vector::multiply(buffer.getWritePointer(chan), FloatType(-1.f), num_samples);
                }
            }
        }

        void process(juce::dsp::AudioBlock<FloatType> block) {
            if (is_on_.load()) {
                const auto num_samples = block.getNumSamples();
                const auto num_channels = block.getNumChannels();
                for (size_t chan = 0; chan < num_channels; ++chan) {
                    zldsp::vector::multiply(block.getChannelPointer(chan), FloatType(-1.f), num_samples);
                }
            }
        }

        void setON(const bool f) { is_on_.store(f); }

    private:
        std::atomic<bool> is_on_;
    };
} // zldsp::phase
