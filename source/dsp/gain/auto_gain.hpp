// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "origin_gain.hpp"

namespace zldsp::gain {
    /**
     * a lock free, thread safe auto-gain class
     * it will hard-clip output signal to 0dB
     * @tparam FloatType
     */
    template<typename FloatType>
    class AutoGain {
    public:
        AutoGain() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            gain_dsp_.prepare(spec, 1.0);
        }

        void prepareBuffer() {
            if (is_on_.load() != c_is_on_) {
                c_is_on_ = is_on_.load();
                gain_dsp_.setGainLinear(1);
                gain_dsp_.reset();
                gain_.store(FloatType(1));
            }
        }

        void processPre(juce::AudioBuffer<FloatType> &buffer) {
            prepareBuffer();
            if (c_is_on_) {
                auto block = juce::dsp::AudioBlock<FloatType>(buffer);
                pre_rms_ = std::max(calculateRMS(block), FloatType(0.000000001));
            }
        }

        void processPre(juce::dsp::AudioBlock<FloatType> block) {
            prepareBuffer();
            if (c_is_on_) {
                pre_rms_ = std::max(calculateRMS(block), FloatType(0.000000001));
            }
        }

        template<bool IsBypassed = false>
        void processPost(juce::AudioBuffer<FloatType> &buffer) {
            if (c_is_on_) {
                auto block = juce::dsp::AudioBlock<FloatType>(buffer);
                processPostBlockInternal<IsBypassed>(block);
            }
        }

        template<bool isBypassed = false>
        void processPost(juce::dsp::AudioBlock<FloatType> block) {
            if (c_is_on_) {
                processPostBlockInternal<isBypassed>(block);
            }
        }

        void setRampDurationSeconds(double newDurationSeconds) {
            gain_dsp_.setRampDurationSeconds(newDurationSeconds);
        }

        void enable(const bool f) {
            if (f) {
                is_on_.store(true);
            } else {
                is_on_.store(false);
                gain_.store(FloatType(1));
            }
        }

        FloatType getGainDecibels() const {
            return juce::Decibels::gainToDecibels(gain_.load());
        }

    private:
        std::atomic<bool> is_on_{false};
        bool c_is_on_{false};
        std::atomic<FloatType> gain_;
        FloatType pre_rms_, post_rms_;
        Gain<FloatType> gain_dsp_;

        template<bool isBypassed = false>
        void processPostBlockInternal(juce::dsp::AudioBlock<FloatType> block) {
            if (pre_rms_ > FloatType(0.00001)) {
                post_rms_ = std::max(calculateRMS(block), FloatType(0.000000001));
                gain_.store(gain_dsp_.getCurrentGainLinear());
                gain_dsp_.setGainLinear(pre_rms_ / post_rms_);
            }
            if (!isBypassed) {
                juce::dsp::ProcessContextReplacing<FloatType> context(block);
                gain_dsp_.template process<false>(block);
                for (size_t channel = 0; channel < block.getNumChannels(); ++channel) {
                    zldsp::vector::clamp(block.getChannelPointer(channel),
                                    static_cast<FloatType>(-1.0),
                                    static_cast<FloatType>(1.0),
                                    static_cast<size_t>(block.getNumSamples()));
                }
            } else {
                gain_dsp_.template process<true>(block);
            }
        }

        FloatType calculateRMS(juce::dsp::AudioBlock<FloatType> block) {
            FloatType ms = 0;
            for (size_t channel = 0; channel < block.getNumChannels(); channel++) {
                auto data = block.getChannelPointer(channel);
                ms += zldsp::vector::sumsqr(data, block.getNumSamples());
            }
            return std::sqrt(ms / static_cast<FloatType>(block.getNumChannels()));
        }
    };
} // zldsp::gain
