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
#include "../../vector/vector.hpp"

namespace zldsp::filter {
    /**
     * a dynamic IIR filter which processes audio on the real-time thread
     * @tparam FloatType the float type of input audio buffer
     */
    template<class FilterType, typename FloatType>
    class DynamicBase {
    public:
        DynamicBase() = default;

        /**
         * reset IIR filter state and follower state
         */
        void reset() {
            filter_.reset();
            follower_.reset(static_cast<FloatType>(0));
        }

        /**
         * reset dynamic status
         */
        void resetDynamic() {
            filter_.setGain(base_gain_);
            follower_.reset(static_cast<FloatType>(0));
        }

        /**
         * prepare the dynamic filter
         * @param sample_rate
         * @param num_channels
         * @param max_num_samples
         */
        void prepare(const double sample_rate, const size_t num_channels, const size_t max_num_samples) {
            filter_.prepare(sample_rate, num_channels, max_num_samples);
            follower_.prepare(sample_rate);
        }

        /**
         * set dynamic filter base gain
         * @param base_gain
         */
        void setBaseGain(const double base_gain) {
            base_gain_ = base_gain;
            gain_diff_ = target_gain_ - base_gain_;
        }

        /**
         * set dynamic filter target gain
         * @param target_gain
         */
        void setTargetGain(const double target_gain) {
            target_gain_ = target_gain;
            gain_diff_ = target_gain_ - base_gain_;
        }

        /**
         * set dynamic filter threshold
         * @param threshold
         */
        template<bool to_update = true>
        void setThreshold(const FloatType threshold) {
            threshold_ = threshold;
            if constexpr (to_update) {
                updateTK();
            }
        }

        /**
         * set dynamic filter knee width
         * @param knee
         */
        template<bool to_update = true>
        void setKnee(const FloatType knee) {
            knee_ = knee;
            if constexpr (to_update) {
                updateTK();
            }
        }

        /**
         *
         * @return the underlying IIR filter
         */
        FilterType &getFilter() {
            return filter_;
        }

        /**
         *
         * @return the underlying follower
         */
        zldsp::compressor::PSFollower<FloatType, true, false> &getFollower() {
            return follower_;
        }

    protected:
        FilterType filter_{};
        zldsp::compressor::PSFollower<FloatType, true, false> follower_;

        bool dynamic_on_{false};
        double base_gain_{}, target_gain_{}, gain_diff_{};

        FloatType threshold_{}, knee_{};
        FloatType threshold_shift_{};
        FloatType low_abs_{}, slope_abs_{};
        FloatType low_sqr_{}, slope_sqr_{};

        /**
         * process the incoming audio buffer
         * @param main_buffer
         * @param side_buffer
         * @param num_samples
         */
        template<bool bypass = false, bool dynamic_on = false, bool dynamic_bypass = false>
        void process(std::span<FloatType *> main_buffer, std::span<FloatType *> side_buffer,
                     const size_t num_samples) {
            if constexpr (dynamic_on) {
                if constexpr (dynamic_bypass) {
                    filter_.template setGain<true>(base_gain_);
                    filter_.updateGain();
                }
                // make sure that freq & q are update to date
                filter_.skipSmooth();
                // calculate portion using SIMD
                auto side_v = kfr::make_univector(side_buffer[0], num_samples);
                if (side_buffer.size() == 1) {
                    side_v = kfr::log10(kfr::max(kfr::abs(side_v), FloatType(1e-12)));
                    side_v = (side_v - low_abs_) * slope_abs_;
                } else {
                    side_v = kfr::sqr(side_v);
                    for (size_t chan = 0; chan < side_buffer.size(); ++chan) {
                        side_v = side_v + kfr::sqr(kfr::make_univector(side_buffer[chan], num_samples));
                    }
                    side_v = kfr::log10(kfr::max(side_v, FloatType(1e-24)));
                    side_v = (side_v - low_sqr_) * slope_sqr_;
                }
                side_v = kfr::clamp(side_v, FloatType(0), FloatType(1));
                // dynamic processing
                if (main_buffer.size() == 1) {
                    const auto main_pointer = main_buffer[0];
                    for (size_t i = 0; i < num_samples; ++i) {
                        if constexpr (dynamic_bypass) {
                            const auto p = side_v[i];
                            follower_.processSample(p * p);
                        } else {
                            const auto p = side_v[i];
                            filter_.template setGain<true>(base_gain_ + follower_.processSample(p * p) * gain_diff_);
                            filter_.updateGain();
                        }
                        if constexpr (bypass) {
                            filter_.processSample(0, main_pointer[i]);
                        } else {
                            main_pointer[i] = filter_.processSample(0, main_pointer[i]);
                        }
                    }
                } else {
                    for (size_t i = 0; i < num_samples; ++i) {
                        if constexpr (dynamic_bypass) {
                            const auto p = side_v[i];
                            follower_.processSample(p * p);
                        } else {
                            const auto p = side_v[i];
                            filter_.template setGain<true>(base_gain_ + follower_.processSample(p * p) * gain_diff_);
                            filter_.updateGain();
                        }
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

        void updateTK() {
            const auto low = threshold_ - knee_;
            const auto slope = static_cast<FloatType>(0.5) / knee_;
            low_abs_ = low / static_cast<FloatType>(20);
            slope_abs_ = slope * static_cast<FloatType>(20);

            low_sqr_ = low / static_cast<FloatType>(10);
            slope_sqr_ = slope * static_cast<FloatType>(10);
        }
    };
}
