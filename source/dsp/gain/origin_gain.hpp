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

#include "../chore/smoothed_value.hpp"
#include "../vector/vector.hpp"

namespace zldsp::gain {
    template<typename FloatType>
    class Gain {
    public:
        Gain() noexcept = default;

        void reset() {
            gain_.setCurrentAndTarget(FloatType(0));
        }

        void setGainLinear(FloatType newGain) noexcept {
            gain_.setTarget(newGain);
        }

        void setGainDecibels(FloatType newGainDecibels) noexcept {
            setGainLinear(juce::Decibels::decibelsToGain<FloatType>(newGainDecibels, (-240)));
        }

        FloatType getTargetGainLinear() const noexcept { return gain_.getTarget(); }

        FloatType getTargetGainDecibels() const noexcept {
            return juce::Decibels::gainToDecibels<FloatType>(getTargetGainLinear(), FloatType(-240));
        }

        FloatType getCurrentGainLinear() const noexcept { return gain_.getCurrent(); }

        FloatType getCurrentGainDecibels() const noexcept {
            return juce::Decibels::gainToDecibels<FloatType>(getCurrentGainLinear(), FloatType(-240));
        }

        [[nodiscard]] bool isSmoothing() const noexcept { return gain_.isSmoothing(); }

        void prepare(const juce::dsp::ProcessSpec &spec, const double rampLengthInSeconds) noexcept {
            gain_.prepare(spec.sampleRate, rampLengthInSeconds);
            gain_vs_.resize(static_cast<size_t>(spec.maximumBlockSize));
        }

        template<bool IsBypassed = false>
        void process(juce::AudioBuffer<FloatType> &buffer) {
            auto block = juce::dsp::AudioBlock<FloatType>(buffer);
            process<IsBypassed>(block);
        }

        template<bool IsBypassed = false>
        void process(juce::dsp::AudioBlock<FloatType> block) {
            if (!gain_.isSmoothing()) {
                if (IsBypassed) return;
                for (size_t chan = 0; chan < block.getNumChannels(); ++chan) {
                    auto *channel_data = block.getChannelPointer(chan);
                    zldsp::vector::multiply(channel_data, gain_.getCurrent(), block.getNumSamples());
                }
            } else {
                for (size_t idx = 0; idx < block.getNumSamples(); ++idx) {
                    gain_vs_[idx] = gain_.getNext();
                }
                if (IsBypassed) return;
                for (size_t chan = 0; chan < block.getNumChannels(); ++chan) {
                    auto *channel_data = block.getChannelPointer(chan);
                    zldsp::vector::multiply(channel_data, gain_vs_.data(), block.getNumSamples());
                }
            }
        }

    private:
        zldsp::chore::SmoothedValue<FloatType, zldsp::chore::SmoothedTypes::FixLin> gain_{FloatType(1)};
        kfr::univector<FloatType> gain_vs_;
    };
}
