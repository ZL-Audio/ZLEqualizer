// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "lufs_meter.hpp"

namespace zldsp::loudness {
    template<typename FloatType, size_t MaxChannels = 2, bool UseLowPass = false>
    class LUFSMatcher {
    public:
        LUFSMatcher() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            pre_loudness_meter_.prepare(spec);
            post_loudness_meter_.prepare(spec);
            sample_rate_ = spec.sampleRate;
            reset();
        }

        void reset() {
            pre_loudness_meter_.reset();
            post_loudness_meter_.reset();
            loudness_diff_.store(FloatType(0));
            current_count_ = 0.0;
        }

        void process(juce::AudioBuffer<FloatType> &pre, juce::AudioBuffer<FloatType> &post) {
            pre_loudness_meter_.process(pre);
            post_loudness_meter_.process(post);
            current_count_ += static_cast<double>(pre.getNumSamples());
            if (current_count_ >= sample_rate_) {
                current_count_ -= sample_rate_;
                const auto pre_loudness = pre_loudness_meter_.getIntegratedLoudness();
                const auto post_loudness = post_loudness_meter_.getIntegratedLoudness();
                loudness_diff_.store(post_loudness - pre_loudness);
            }
        }

        FloatType getDiff() const {
            return loudness_diff_.load();
        }

    private:
        LUFSMeter<FloatType, MaxChannels, UseLowPass> pre_loudness_meter_, post_loudness_meter_;
        std::atomic<FloatType> loudness_diff_{FloatType(0)};
        double sample_rate_{48000}, current_count_{0};
    };
}
