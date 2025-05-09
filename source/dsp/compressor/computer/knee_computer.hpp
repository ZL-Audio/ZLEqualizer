// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace zldsp::compressor {
    template<typename FloatType>
    struct LinearCurve {
        static constexpr FloatType a{FloatType(0)};
        FloatType b, c;

        void setPara(FloatType t, FloatType r, FloatType w) {
            juce::ignoreUnused(w);
            b = FloatType(1) / r;
            c = t * (FloatType(1) - FloatType(1) / r);
        }
    };

    template<typename FloatType>
    struct DownCurve {
        FloatType a, c;
        static constexpr FloatType b{FloatType(0)};

        void setPara(FloatType t, FloatType r, FloatType w) {
            a = FloatType(0.5) / (r * std::min(t + w, FloatType(-0.0001)));
            c = FloatType(0.5) * (w - t) / r + t;
        }
    };

    template<typename FloatType>
    struct UpCurve {
        FloatType a, c;
        static constexpr FloatType b{FloatType(1)};

        void setPara(FloatType t, FloatType r, FloatType w) {
            a = FloatType(0.5) * (FloatType(1) - r) / (r * std::min(t + w, FloatType(-0.0001)));
            c = FloatType(0.5) * (FloatType(1) - r) * (w - t) / r;
        }
    };

    /**
     * a computer that computes the current compression
     * @tparam FloatType
     * @tparam UseCurve whether to use curve
     * @tparam UseBound whether to use bound
     */
    template<typename FloatType, bool UseCurve = false, bool UseBound = false>
    class KneeComputer final {
    public:
        KneeComputer() = default;

        void prepareBuffer() {
            if (to_interpolate_.exchange(false)) {
                interpolate();
            }
        }

        FloatType eval(FloatType x) {
            if (x <= low_th_) {
                return x;
            } else if (x >= high_th_) {
                const auto y = UseCurve ? paras_[2] + paras_[3] * x + paras_[4] * x * x : paras_[2] + paras_[3] * x;
                return UseBound ? std::max(x - c_bound_, y) : y;
            } else {
                const auto xx = x + paras_[1];
                const auto y = x + paras_[0] * xx * xx;
                return UseBound ? std::max(x - c_bound_, y) : y;
            }
        }

        /**
         * computes the current compression
         * @param x input level (in dB)
         * @return current compression (in dB)
         */
        FloatType process(FloatType x) {
            return eval(x) - x;
        }

        inline void setThreshold(FloatType v) {
            threshold_.store(v);
            to_interpolate_.store(true);
        }

        inline FloatType getThreshold() const { return threshold_.load(); }

        inline void setRatio(FloatType v) {
            ratio_.store(std::max(FloatType(1), v));
            to_interpolate_.store(true);
        }

        inline FloatType getRatio() const { return ratio_.load(); }

        inline void setKneeW(FloatType v) {
            knee_w_.store(std::max(v, FloatType(0.01)));
            to_interpolate_.store(true);
        }

        inline FloatType getKneeW() const { return knee_w_.load(); }

        inline void setCurve(FloatType v) {
            curve_.store(juce::jlimit(FloatType(-1), FloatType(1), v));
            to_interpolate_.store(true);
        }

        inline FloatType getCurve() const { return curve_.load(); }

        inline void setBound(FloatType v) {
            bound_.store(v);
        }

        inline FloatType getBound() const { return bound_.load(); }

        FloatType getReductionAtKnee() const { return reduction_at_knee_; }

    private:
        LinearCurve<FloatType> linear_curve_;
        DownCurve<FloatType> down_curve_;
        UpCurve<FloatType> up_curve_;
        std::atomic<FloatType> threshold_{-18}, ratio_{2};
        std::atomic<FloatType> knee_w_{FloatType(0.25)}, curve_{0};
        FloatType low_th_{0}, high_th_{0};
        std::atomic<FloatType> bound_{60};
        FloatType c_bound_{60};
        FloatType reduction_at_knee_{FloatType(0.001)};
        std::array<FloatType, 5> paras_;
        std::atomic<bool> to_interpolate_{true};

        void interpolate() {
            const auto currentThreshold = threshold_.load();
            const auto currentKneeW = knee_w_.load();
            const auto currentRatio = ratio_.load();
            if (UseBound) {
                c_bound_ = bound_.load();
            }
            const auto currentCurve = curve_.load();
            low_th_ = currentThreshold - currentKneeW;
            high_th_ = currentThreshold + currentKneeW;
            paras_[0] = FloatType(1) / currentRatio - FloatType(1);
            paras_[1] = -low_th_;
            paras_[0] *= FloatType(1) / (currentKneeW * FloatType(4));
            if (UseCurve) {
                if (currentCurve >= FloatType(0)) {
                    const auto alpha = FloatType(1) - currentCurve, beta = currentCurve;
                    linear_curve_.setPara(currentThreshold, currentRatio, currentKneeW);
                    down_curve_.setPara(currentThreshold, currentRatio, currentKneeW);
                    paras_[2] = alpha * linear_curve_.c + beta * down_curve_.c;
                    paras_[3] = alpha * linear_curve_.b + beta * down_curve_.b;
                    paras_[4] = alpha * linear_curve_.a + beta * down_curve_.a;
                } else {
                    const auto alpha = FloatType(1) + currentCurve, beta = -currentCurve;
                    linear_curve_.setPara(currentThreshold, currentRatio, currentKneeW);
                    up_curve_.setPara(currentThreshold, currentRatio, currentKneeW);
                    paras_[2] = alpha * linear_curve_.c + beta * up_curve_.c;
                    paras_[3] = alpha * linear_curve_.b + beta * up_curve_.b;
                    paras_[4] = alpha * linear_curve_.a + beta * up_curve_.a;
                }
            } else {
                linear_curve_.setPara(currentThreshold, currentRatio, currentKneeW);
                paras_[2] = linear_curve_.c;
                paras_[3] = linear_curve_.b;
            }
            reduction_at_knee_ = std::max(FloatType(0.001), high_th_ - eval(high_th_));
        }
    };
} // KneeComputer
