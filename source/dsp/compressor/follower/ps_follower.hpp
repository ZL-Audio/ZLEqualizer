// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <numbers>

namespace zldsp::compressor {
    /**
     * a punch-smooth follower
     * @tparam FloatType
     * @tparam UseSmooth whether to use smooth
     * @tparam UsePunch whether to use punch
     */
    template<typename FloatType, bool UseSmooth = false, bool UsePunch = false>
    class PSFollower {
    public:
        PSFollower() = default;

        /**
         * call before processing starts
         * @tparam ToReset whether to reset the internal state
         * @param sr sampleRate
         */
        template<bool ToReset = true>
        void prepare(const double sr) {
            exp_factor_ = -2.0 * std::numbers::pi * 1000.0 / sr;
            if (ToReset) {
                state_ = FloatType(0);
                y_ = FloatType(0);
            }
            to_update_.store(true);
        }

        /**
         * update values before processing a buffer
         */
        void prepareBuffer() {
            if (to_update_.exchange(false)) {
                update();
            }
        }

        /**
         * process a sample
         * @param x
         * @return
         */
        FloatType processSample(const FloatType x) {
            FloatType y0;
            if (UseSmooth) {
                state_ = std::max(x, release_ * state_ + release_c_ * x);
                const auto y1 = attack_ * y_ + attack_c_ * state_;
                const auto y2 = x >= y_ ? attack_ * y_ + attack_c_ * x : release_ * y_ + release_c_ * x;
                y0 = smooth_ * y1 + smooth_c_ * y2;
            } else {
                y0 = x >= y_ ? attack_ * y_ + attack_c_ * x : release_ * y_ + release_c_ * x;
            }
            if (UsePunch) {
                const auto slope0 = y0 - y_;
                if (punch_ >= FloatType(0)) {
                    if (slope0 < slope_) {
                        slope_ = punch_ * slope_ + punch_c_ * slope0;
                    } else {
                        slope_ = slope0;
                    }
                } else {
                    if (slope0 > slope_ && slope_ >= FloatType(0)) {
                        slope_ = punch_ * slope_ + punch_c_ * slope0;
                    } else {
                        slope_ = slope0;
                    }
                }
                y_ += slope_;
            } else {
                y_ = y0;
            }
            return y_;
        }

        void setAttack(const FloatType millisecond) {
            attack_time_.store(std::max(FloatType(0), millisecond));
            to_update_.store(true);
        }

        void setRelease(const FloatType millisecond) {
            release_time_.store(std::max(FloatType(0), millisecond));
            to_update_.store(true);
        }

        void setPunch(const FloatType x) {
            punch_portion_.store(std::clamp(x, FloatType(-1), FloatType(1)));
            to_update_.store(true);
        }

        void setSmooth(const FloatType x) {
            smooth_portion_.store(std::clamp(x, FloatType(0), FloatType(1)));
            to_update_.store(true);
        }

    private:
        FloatType y_{}, state_{}, slope_{};
        FloatType attack_{}, attack_c_{}, release_{}, release_c_{};
        FloatType punch_{}, punch_c_{}, smooth_{}, smooth_c_{};
        double exp_factor_{-0.1308996938995747};
        std::atomic<FloatType> attack_time_{1}, release_time_{1}, smooth_portion_{0}, punch_portion_{0};
        std::atomic<bool> to_update_{true};

        void update() {
            // cache atomic values
            const auto current_attack_time = static_cast<double>(attack_time_.load());
            const auto current_release_time = static_cast<double>(release_time_.load());
            const auto current_smooth_portion = static_cast<double>(smooth_portion_.load());
            const auto current_punch_portion = static_cast<double>(punch_portion_.load());
            // update attack
            if (current_attack_time < 0.001) {
                attack_ = FloatType(0);
            } else {
                if (UsePunch) {
                    attack_ = static_cast<FloatType>(std::exp(
                        exp_factor_ / current_attack_time / (1. - std::pow(std::abs(current_punch_portion), 2.) * 0.125)));
                } else {
                    attack_ = static_cast<FloatType>(std::exp(exp_factor_ / current_attack_time));
                }
            }
            attack_c_ = FloatType(1) - attack_;
            // update release
            if (current_release_time < 0.001) {
                release_ = FloatType(0);
            } else {
                release_ = static_cast<FloatType>(std::exp(exp_factor_ / current_release_time));
            }
            release_c_ = FloatType(1) - release_;
            if (UseSmooth) {
                // update smooth
                smooth_ = static_cast<FloatType>(current_smooth_portion);
                smooth_c_ = FloatType(1) - smooth_;
            }
            if (UsePunch) {
                // update punch
                if (current_attack_time < 0.001 || std::abs(current_punch_portion) < 0.001) {
                    punch_ = FloatType(0);
                } else {
                    punch_ = static_cast<FloatType>(std::exp(
                        exp_factor_ / current_attack_time / std::abs(current_punch_portion)));
                }
                punch_c_ = FloatType(1) - punch_;
            }
        }
    };
}
