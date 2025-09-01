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
#include <vector>
#include <array>
#include <algorithm>

#include "computer_base.hpp"

namespace zldsp::compressor {
    /**
     * a computer that computes the current compression
     * @tparam FloatType
     */
    template<typename FloatType, bool kOutputDiff = false>
    class KneeComputer final : public ComputerBase<FloatType> {
    public:
        KneeComputer() = default;

        bool prepareBuffer() override {
            if (to_interpolate_.exchange(false, std::memory_order::acquire)) {
                interpolate();
                return true;
            }
            return false;
        }

        void copyFrom(KneeComputer &other) {
            low_th_ = other.low_th_;
            high_th_ = other.high_th_;
            para_mid_g0_ = other.para_mid_g0_;
            para_high_g0_ = other.para_high_g0_;
            para_over_g0_ = other.para_over_g0_;
        }

        FloatType eval(FloatType x) override {
            if (x <= low_th_) {
                if constexpr (kOutputDiff) {
                    return FloatType(0);
                } else {
                    return x;
                }
            } else if (x < high_th_) {
                return (para_mid_g0_[0] * x + para_mid_g0_[1]) * x + para_mid_g0_[2];
            } else if (x < FloatType(0)) {
                return (para_high_g0_[0] * x + para_high_g0_[1]) * x + para_high_g0_[2];
            } else {
                return para_over_g0_[0] * x + para_over_g0_[1];
            }
        }

        inline void setThreshold(FloatType v) {
            threshold_.store(v, std::memory_order::relaxed);
            to_interpolate_.store(true, std::memory_order::release);
        }

        inline FloatType getThreshold() const { return threshold_.load(std::memory_order::relaxed); }

        inline void setRatio(FloatType v) {
            ratio_.store(std::max(FloatType(1), v), std::memory_order::relaxed);
            to_interpolate_.store(true, std::memory_order::release);
        }

        inline FloatType getRatio() const { return ratio_.load(std::memory_order::relaxed); }

        inline void setKneeW(FloatType v) {
            knee_w_.store(std::max(v, FloatType(0.01)), std::memory_order::relaxed);
            to_interpolate_.store(true, std::memory_order::release);
        }

        inline FloatType getKneeW() const { return knee_w_.load(std::memory_order::relaxed); }

        inline void setCurve(FloatType v) {
            curve_.store(std::clamp(v, FloatType(-1), FloatType(1)), std::memory_order::relaxed);
            to_interpolate_.store(true, std::memory_order::release);
        }

        inline FloatType getCurve() const { return curve_.load(std::memory_order::relaxed); }

    private:
        struct LinearCurve {
            FloatType b, c;

            void setPara(FloatType t, FloatType r, FloatType) {
                b = FloatType(1) / r;
                c = t * (FloatType(1) - b);
            }
        };

        struct DownCurve {
            FloatType a, c;
            static constexpr FloatType b{FloatType(0)};

            void setPara(FloatType t, FloatType r, FloatType w) {
                const auto temp = FloatType(0.5) / r;
                a = temp / std::min(t + w, FloatType(-0.0001));
                c = temp * (w - t) + t;
            }
        };

        struct UpCurve {
            FloatType a, c;
            static constexpr FloatType b{FloatType(1)};

            void setPara(FloatType t, FloatType r, FloatType w) {
                const auto temp = FloatType(0.5) * (FloatType(1) - r) / r;
                a = temp / std::min(t + w, FloatType(-0.0001));
                c = temp * (w - t);
            }
        };

        LinearCurve linear_curve_;
        DownCurve down_curve_;
        UpCurve up_curve_;
        std::atomic<FloatType> threshold_{-18}, ratio_{2};
        std::atomic<FloatType> knee_w_{FloatType(0.25)}, curve_{0};
        FloatType low_th_{0}, high_th_{0};
        std::array<FloatType, 3> para_mid_g0_{}, para_high_g0_{};
        std::array<FloatType, 2> para_over_g0_{};
        std::atomic<bool> to_interpolate_{true};

        void interpolate() {
            const auto current_threshold = threshold_.load(std::memory_order::relaxed);
            const auto current_knee_w = knee_w_.load(std::memory_order::relaxed);
            const auto current_ratio = ratio_.load(std::memory_order::relaxed);
            const auto current_curve = curve_.load(std::memory_order::relaxed);
            low_th_ = current_threshold - current_knee_w;
            high_th_ = current_threshold + current_knee_w; {
                // update mid curve parameters
                const auto a0 = (FloatType(1) / current_ratio - FloatType(1)) / (current_knee_w * FloatType(4));
                const auto a1 = -low_th_;
                para_mid_g0_[0] = a0;
                const auto a0a1 = a0 * a1;
                if constexpr (kOutputDiff) {
                    para_mid_g0_[1] = FloatType(2) * a0a1;
                } else {
                    para_mid_g0_[1] = FloatType(2) * a0a1 + FloatType(1);
                }
                para_mid_g0_[2] = a0a1 * a1;
            } {
                if (current_curve >= FloatType(0)) {
                    const auto alpha = FloatType(1) - current_curve, beta = current_curve;
                    linear_curve_.setPara(current_threshold, current_ratio, current_knee_w);
                    down_curve_.setPara(current_threshold, current_ratio, current_knee_w);
                    para_high_g0_[2] = alpha * linear_curve_.c + beta * down_curve_.c;
                    para_high_g0_[1] = alpha * linear_curve_.b + beta * down_curve_.b;
                    para_high_g0_[0] = beta * down_curve_.a;
                } else {
                    const auto alpha = FloatType(1) + current_curve, beta = -current_curve;
                    linear_curve_.setPara(current_threshold, current_ratio, current_knee_w);
                    up_curve_.setPara(current_threshold, current_ratio, current_knee_w);
                    para_high_g0_[2] = alpha * linear_curve_.c + beta * up_curve_.c;
                    para_high_g0_[1] = alpha * linear_curve_.b + beta * up_curve_.b;
                    para_high_g0_[0] = beta * up_curve_.a;
                }
                if constexpr (kOutputDiff) {
                    para_high_g0_[1] -= FloatType(1);
                }
            } {
                if (high_th_ <= FloatType(0)) {
                    para_over_g0_[0] = para_high_g0_[1];
                    para_over_g0_[1] = para_high_g0_[2];
                } else {
                    para_over_g0_[0] = linear_curve_.b;
                    para_over_g0_[1] = linear_curve_.c;
                    if constexpr (kOutputDiff) {
                        para_over_g0_[0] -= FloatType(1);
                    }
                }
            }
        }
    };
} // KneeComputer
