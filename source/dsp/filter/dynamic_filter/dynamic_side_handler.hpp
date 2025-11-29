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
#include "../../vector/vector.hpp"

namespace zldsp::filter {
    /**
     * a dynamic side-chain handler
     * @tparam FloatType the float type of input audio buffer
     */
    template <typename FloatType>
    class DynamicSideHandler {
    public:
        explicit DynamicSideHandler() = default;

        void reset() {
            follower_.reset(static_cast<FloatType>(0));
        }

        void prepare(const double sample_rate) {
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
        template <bool to_update = true>
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
        template <bool to_update = true>
        void setKnee(const FloatType knee) {
            knee_ = std::max(knee, static_cast<FloatType>(0.01));
            if constexpr (to_update) {
                updateTK();
            }
        }

        void process(std::span<FloatType*> side_buffer, const size_t num_samples) {
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
            side_v = kfr::sqr(kfr::clamp(side_v, FloatType(0), FloatType(1)));
        }

        /**
         *
         * @return the underlying follower
         */
        zldsp::compressor::PSFollower<FloatType>& getFollower() {
            return follower_;
        }

        [[nodiscard]] double getBaseGain() const {
            return base_gain_;
        }

        double getCurrentGain() {
            return base_gain_ + follower_.getCurrentSample() * gain_diff_;
        }

        template <zldsp::compressor::SState s_state>
        double getNextGain(const double p) {
            return base_gain_ + follower_.template processSample<s_state>(p) * gain_diff_;
        }

    protected:
        zldsp::compressor::PSFollower<FloatType> follower_;
        double base_gain_{}, target_gain_{}, gain_diff_{};

        FloatType threshold_{}, knee_{static_cast<FloatType>(0.01)};
        FloatType low_abs_{}, slope_abs_{};
        FloatType low_sqr_{}, slope_sqr_{};

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
