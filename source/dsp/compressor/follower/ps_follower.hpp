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
#include <numbers>
#include <cmath>
#include <algorithm>

#include "follower_base.hpp"

namespace zldsp::compressor {
    enum PPState { kOff, kPunch, kPump };

    /**
     * a punch-smooth follower
     * @tparam FloatType
     * @tparam kUseSmooth whether to use smooth
     * @tparam kUsePP whether to use pump-punch
     */
    template<typename FloatType, bool kUseSmooth = false, bool kUsePP = false>
    class PSFollower final : public FollowerBase<FloatType> {
    public:
        PSFollower() = default;

        /**
         * call before processing starts
         * @param sr sampleRate
         */
        void prepare(const double sr) override {
            exp_factor_ = -2.0 * std::numbers::pi * 1000.0 / sr;
            update();
        }

        /**
         * reset the follower
         */
        void reset(const FloatType x) override {
            y_ = x;
            state_ = x;
            slope_ = FloatType(0);
        }

        /**
         * update values before processing a buffer by copying parameters from another follower
         */
        void copyFrom(PSFollower &other) {
            attack_ = other.attack_;
            release_ = other.release_;
            if constexpr (kUseSmooth) {
                smooth_ = other.smooth_;
            }
            if constexpr (kUsePP) {
                pp_state_ = other.pp_state_;
                pp_ = other.pp_;
            }
        }

        FloatType processSample(const FloatType x) override {
            FloatType y0;
            if constexpr (kUseSmooth) {
                state_ = std::max(x, release_ * (state_ - x) + x);
                const auto y1 = attack_ * (y_ - state_) + state_;
                const auto y2 = x >= y_ ? attack_ * (y_ - x) + x : release_ * (y_ - x) + x;
                y0 = smooth_ * (y1 - y2) + y2;
            } else {
                state_ = std::max(x, release_ * (state_ - x) + x);
                y0 = attack_ * (y_ - state_) + state_;
            }
            if constexpr (kUsePP) {
                const auto slope0 = y0 - y_;
                switch (pp_state_) {
                    case PPState::kOff: {
                        slope_ = slope0;
                        y_ = y0;
                        break;
                    }
                    case PPState::kPump: {
                        if (slope0 < slope_) {
                            slope_ = pp_ * (slope_ - slope0) + slope0;
                        } else {
                            slope_ = slope0;
                        }
                        y_ += slope_;
                        break;
                    }
                    case PPState::kPunch: {
                        if (slope0 > slope_ && slope_ >= FloatType(0)) {
                            slope_ = pp_ * (slope_ - slope0) + slope0;
                        } else {
                            slope_ = slope0;
                        }
                        y_ += slope_;
                        break;
                    }
                }
            } else {
                y_ = y0;
            }
            return y_;
        }

        template<bool to_update = true>
        void setAttack(const FloatType millisecond) {
            attack_time_ = millisecond;
            if constexpr (to_update) {
                update();
            }
        }

        template<bool to_update = true>
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
        template<bool to_update = true>
        void setPumpPunch(const FloatType x) {
            pp_portion_ = x;
            if constexpr (to_update) {
                update();
            }
        }

        /**
         *
         * @param x a float between 0.0 and 1.0
         */
        template<bool to_update = true>
        void setSmooth(const FloatType x) {
            smooth_portion_ = x;
            if constexpr (to_update) {
                update();
            }
        }

    private:
        FloatType y_{}, state_{}, slope_{};
        FloatType attack_{}, release_{};

        PPState pp_state_{PPState::kOff};
        FloatType pp_{}, smooth_{};

        double exp_factor_{-0.1308996938995747};
        FloatType attack_time_{50}, release_time_{100}, pp_portion_{0}, smooth_portion_{0};

        void update() {
            // cache atomic values
            // update attack
            if (attack_time_ < 0.001) {
                attack_ = FloatType(0);
            } else {
                if constexpr (kUsePP) {
                    attack_ = static_cast<FloatType>(std::exp(
                        exp_factor_ / attack_time_ / (
                            1. - std::pow(std::abs(pp_portion_), 2.) * 0.125)));
                } else {
                    attack_ = static_cast<FloatType>(std::exp(exp_factor_ / attack_time_));
                }
            }
            // update release
            if (release_time_ < 0.001) {
                release_ = FloatType(0);
            } else {
                release_ = static_cast<FloatType>(std::exp(exp_factor_ / release_time_));
            }
            if constexpr (kUseSmooth) {
                // update smooth
                smooth_ = static_cast<FloatType>(smooth_portion_);
            }
            if constexpr (kUsePP) {
                // update pump-punch
                if (attack_time_ < 0.001 || std::abs(pp_portion_) < 0.001) {
                    pp_ = FloatType(0);
                    pp_state_ = PPState::kOff;
                } else {
                    pp_state_ = pp_portion_ > 0 ? PPState::kPump : PPState::kPunch;
                    pp_ = static_cast<FloatType>(std::exp(
                        exp_factor_ / attack_time_ / std::abs(pp_portion_)));
                }
            }
        }
    };
}
