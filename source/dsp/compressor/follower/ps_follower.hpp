// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <numbers>
#include <cmath>
#include <algorithm>

namespace zldsp::compressor {
    enum class SState { kOff, kFull, kMix };

    /**
     * a punch-smooth follower
     * @tparam FloatType
     */
    template <typename FloatType>
    class PSFollower final {
    public:
        PSFollower() = default;

        /**
         * call before processing starts
         * @param sr sampleRate
         */
        void prepare(const double sr) {
            exp_factor_ = -2.0 * std::numbers::pi * 1000.0 / sr;
            update();
        }

        /**
         * reset the follower
         */
        void reset(const FloatType x) {
            y_ = x;
            state_ = x;
            slope_ = FloatType(0);
        }

        /**
         * update values before processing a buffer by copying parameters from another follower
         */
        void copyFrom(PSFollower& other) {
            attack_ = other.attack_;
            release_ = other.release_;
            smooth_ = other.smooth_;
        }

        template <SState s_state = SState::kOff>
        FloatType processSample(const FloatType x) {
            if constexpr (s_state == SState::kOff) {
                y_ = x >= y_ ? attack_ * (y_ - x) + x : release_ * (y_ - x) + x;
            } else if constexpr (s_state == SState::kFull) {
                state_ = std::max(x, release_ * (state_ - x) + x);
                y_ = attack_ * (y_ - state_) + state_;
            } else {
                state_ = std::max(x, release_ * (state_ - x) + x);
                const auto y1 = attack_ * (y_ - state_) + state_;
                const auto y2 = x >= y_ ? attack_ * (y_ - x) + x : release_ * (y_ - x) + x;
                y_ = smooth_ * (y1 - y2) + y2;
            }
            return y_;
        }

        template <bool to_update = true>
        void setAttack(const FloatType millisecond) {
            attack_time_ = millisecond;
            if constexpr (to_update) {
                update();
            }
        }

        template <bool to_update = true>
        void setRelease(const FloatType millisecond) {
            release_time_ = millisecond;
            if constexpr (to_update) {
                update();
            }
        }

        /**
         *
         * @param x a float between 0.0 and 1.0
         */
        template <bool to_update = true>
        void setSmooth(const FloatType x) {
            smooth_portion_ = x;
            if constexpr (to_update) {
                update();
            }
        }

        FloatType getCurrentSample() const {
            return y_;
        }

        [[nodiscard]] SState getSState() const {
            return s_state_;
        }

    private:
        FloatType y_{}, state_{}, slope_{};
        FloatType attack_{}, release_{};

        SState s_state_{SState::kOff};
        FloatType smooth_{};

        double exp_factor_{-0.1308996938995747};
        FloatType attack_time_{50}, release_time_{100}, smooth_portion_{0};

        void update() {
            // update attack
            if (attack_time_ < 0.001) {
                attack_ = FloatType(0);
            } else {
                attack_ = static_cast<FloatType>(std::exp(exp_factor_ / attack_time_));
            }
            // update release
            if (release_time_ < 0.001) {
                release_ = FloatType(0);
            } else {
                release_ = static_cast<FloatType>(std::exp(exp_factor_ / release_time_));
            }
            smooth_ = static_cast<FloatType>(smooth_portion_);
            if (smooth_ < 0.0001) {
                s_state_ = SState::kOff;
            } else if (smooth_ > 0.9999) {
                s_state_ = SState::kFull;
            } else {
                s_state_ = SState::kMix;
            }
        }
    };
}
