// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>

#include "../../compressor/follower/ps_follower.hpp"
#include "../../chore/decibels.hpp"

namespace zldsp::filter {
    /**
     * a dynamic IIR filter which processes audio on the real-time thread
     * @tparam FloatType the float type of input audio buffer
     */
    template<class FilterType, typename FloatType>
    class DynamicBase {
    public:
        DynamicBase() = default;

        void reset() {
            filter_.reset();
            follower_.reset();
        }

        /**
         * cache dynamic parameters for upcoming audio buffer
         * @param base_gain
         * @param threshold_shift
         */
        void prepareBuffer(const double base_gain, const FloatType threshold_shift) {
            follower_.prepareBuffer();
            c_base_gain_ = base_gain;
            c_gain_diff_ = target_gain_.load(std::memory_order::relaxed) - c_base_gain_;
            if (to_update_tk_.exchange(false, std::memory_order::acquire)
                || std::abs(c_threshold_shift_ - threshold_shift) > static_cast<FloatType>(1e-3)) {
                c_threshold_shift_ = threshold_shift;
                updateThreshold(threshold_shift);
            }
        }

        void prepare(const double sample_rate, const size_t num_channels, const size_t max_num_samples) {
            filter_.prepare(sample_rate, num_channels, max_num_samples);
            follower_.prepare(sample_rate);
        }

        void setDynamicON(const bool f) {
            c_dynamic_on_ = f;
        }

        void setTargetGain(const double target_gain) {
            target_gain_.store(target_gain, std::memory_order::relaxed);
        }

        void setThreshold(const FloatType threshold) {
            threshold_.store(threshold, std::memory_order::relaxed);
            to_update_tk_.store(true, std::memory_order::release);
        }

        void setKnee(const FloatType knee) {
            knee_.store(knee, std::memory_order::relaxed);
            to_update_tk_.store(true, std::memory_order::release);
        }

        FilterType &getFilter() {
            return filter_;
        }

        zldsp::compressor::PSFollower<FloatType, true, false> &getFollower() {
            return follower_;
        }

    protected:
        FilterType filter_{};
        zldsp::compressor::PSFollower<FloatType, true, false> follower_;

        bool c_dynamic_on_{false};

        std::atomic<double> target_gain_{};
        double c_base_gain_{}, c_gain_diff_{};

        std::atomic<bool> to_update_tk_{false};
        std::atomic<FloatType> threshold_, knee_;
        FloatType low_{}, slope_{}, c_threshold_shift_{};

        /**
         * process the incoming audio buffer
         * @tparam bypass
         * @param main_buffer
         * @param side_buffer
         * @param num_samples
         */
        template<bool bypass = false>
        void process(std::span<FloatType *> main_buffer, std::span<FloatType *> side_buffer,
                     const size_t num_samples) {
            if (c_dynamic_on_) {
                if (main_buffer.size() == 1) {
                    const auto main_pointer = main_buffer[0];
                    const auto side_pointer = side_buffer[0];
                    for (size_t i = 0; i < num_samples; ++i) {
                        const auto side_db = zldsp::chore::gainToDecibels(std::abs(side_pointer[i]));
                        const auto p = std::clamp(
                            (side_db - low_) * slope_, static_cast<FloatType>(0), static_cast<FloatType>(1));
                        filter_.template setGain<true>(c_base_gain_ + follower_.processSample(p * p) * c_gain_diff_);
                        filter_.updateGain();
                        if constexpr (bypass) {
                            filter_.processSample(0, main_pointer[i]);
                        } else {
                            main_pointer[i] = filter_.processSample(0, main_pointer[i]);
                        }
                    }
                } else {
                    for (size_t i = 0; i < num_samples; ++i) {
                        FloatType sum_sqr = 0;
                        for (size_t chan = 0; chan < main_buffer.size(); ++chan) {
                            const auto sample = side_buffer[chan][i];
                            sum_sqr += sample * sample;
                        }
                        const auto side_db = zldsp::chore::squareGainToDecibels(sum_sqr);
                        const auto p = std::clamp(
                            (side_db - low_) * slope_, static_cast<FloatType>(0), static_cast<FloatType>(1));
                        filter_.template setGain<true>(c_base_gain_ + follower_.processSample(p * p) * c_gain_diff_);
                        filter_.updateGain();
                        for (size_t chan = 0; chan < main_buffer.size(); ++chan) {
                            if constexpr (bypass) {
                                filter_.processSample(chan, main_buffer[chan][i]);
                            } else {
                                main_buffer[chan][i] = filter_.processSample(chan, main_buffer[chan][i]);
                            }
                        }
                    }
                }
            } else {
                filter_.template process<bypass>(main_buffer, num_samples);
            }
        }

        void updateThreshold(const FloatType threshold_shift) {
            const auto threshold = threshold_.load(std::memory_order_relaxed) + threshold_shift;
            const auto knee = knee_.load(std::memory_order_relaxed);
            low_ = threshold - knee;
            slope_ = static_cast<FloatType>(0.5) / knee;
        }
    };
}
