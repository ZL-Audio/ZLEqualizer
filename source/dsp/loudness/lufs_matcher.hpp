// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include "lufs_meter.hpp"

namespace zldsp::loudness {
    /**
     * a integrated matcher which matches the loudness of pre and post
     * @tparam FloatType the float type of input audio buffer
     * @tparam kUseLowPass whether to use an extra lowpass filter at 22,000 Hz
     */
    template <typename FloatType, bool kUseLowPass = false>
    class LUFSMatcher {
    public:
        LUFSMatcher() = default;

        void prepare(const double sample_rate, const size_t num_channels) {
            pre_loudness_meter_.prepare(sample_rate, num_channels);
            post_loudness_meter_.prepare(sample_rate, num_channels);
            sample_rate_ = sample_rate;
            reset();
        }

        void reset() {
            pre_loudness_meter_.reset();
            post_loudness_meter_.reset();
            loudness_diff_.store(FloatType(0), std::memory_order::relaxed);
            current_count_ = 0.0;
        }

        /**
         * process the pre audio input
         * @param pre
         * @param num_samples
         */
        void processPre(std::span<FloatType*> pre, const size_t num_samples) {
            pre_loudness_meter_.process(pre, num_samples);
        }

        /**
         * process the post audio input
         * @param post
         * @param num_samples
         */
        void processPost(std::span<FloatType*> post, const size_t num_samples) {
            post_loudness_meter_.process(post, num_samples);
            current_count_ += static_cast<double>(num_samples);
            while (current_count_ >= sample_rate_) {
                current_count_ -= sample_rate_;
                const auto pre_loudness = pre_loudness_meter_.getIntegratedLoudness();
                const auto post_loudness = post_loudness_meter_.getIntegratedLoudness();
                loudness_diff_.store(post_loudness - pre_loudness, std::memory_order::relaxed);
            }
        }

        /**
         * thread-safe method
         * get the value of post audio loudness - pre audio loudness
         * @return
         */
        FloatType getDiff() const {
            return loudness_diff_.load(std::memory_order::relaxed);
        }

    private:
        LUFSMeter<FloatType, kUseLowPass> pre_loudness_meter_, post_loudness_meter_;
        std::atomic<FloatType> loudness_diff_{FloatType(0)};
        double sample_rate_{48000}, current_count_{0};
    };
}
