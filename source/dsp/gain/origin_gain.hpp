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

namespace zlGain {
    template<typename FloatType>
    class OriginGain {
    public:
        OriginGain() noexcept = default;

        void reset() {
            gain.setCurrentAndTarget(FloatType(0));
        }

        void setGainLinear(FloatType newGain) noexcept {
            gain.setTarget(newGain);
        }

        void setGainDecibels(FloatType newGainDecibels) noexcept {
            setGainLinear(juce::Decibels::decibelsToGain<FloatType>(newGainDecibels, (-240)));
        }

        FloatType getTargetGainLinear() const noexcept { return gain.getTarget(); }

        FloatType getTargetGainDecibels() const noexcept {
            return juce::Decibels::gainToDecibels<FloatType>(getTargetGainLinear(), FloatType(-240));
        }

        FloatType getCurrentGainLinear() const noexcept { return gain.getCurrent(); }

        FloatType getCurrentGainDecibels() const noexcept {
            return juce::Decibels::gainToDecibels<FloatType>(getCurrentGainLinear(), FloatType(-240));
        }

        [[nodiscard]] bool isSmoothing() const noexcept { return gain.isSmoothing(); }

        void prepare(const juce::dsp::ProcessSpec &spec, const double rampLengthInSeconds) noexcept {
            gain.prepare(spec.sampleRate, rampLengthInSeconds);
            gainVs.resize(static_cast<size_t>(spec.maximumBlockSize));
        }

        template<bool isBypassed = false>
        void process(juce::AudioBuffer<FloatType> &buffer) {
            auto block = juce::dsp::AudioBlock<FloatType>(buffer);
            process<isBypassed>(block);
        }

        template<bool isBypassed = false>
        void process(juce::dsp::AudioBlock<FloatType> block) {
            if (!gain.isSmoothing()) {
                if (isBypassed) return;
                for (size_t chan = 0; chan < block.getNumChannels(); ++chan) {
                    auto *channelData = block.getChannelPointer(chan);
                    zlVector::multiply(channelData, gain.getCurrent(), block.getNumSamples());
                }
            } else {
                for (size_t idx = 0; idx < block.getNumSamples(); ++idx) {
                    gainVs[idx] = gain.getNext();
                }
                if (isBypassed) return;
                for (size_t chan = 0; chan < block.getNumChannels(); ++chan) {
                    auto *channelData = block.getChannelPointer(chan);
                    zlVector::multiply(channelData,  gainVs.data(), block.getNumSamples());
                }
            }
        }

    private:
        zlChore::SmoothedValue<FloatType, zlChore::SmoothedTypes::FixLin> gain;
        std::vector<FloatType> gainVs;
    };
}
