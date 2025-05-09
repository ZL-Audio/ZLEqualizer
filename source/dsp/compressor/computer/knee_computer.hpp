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
            if (to_interpolate.exchange(false)) {
                interpolate();
            }
        }

        FloatType eval(FloatType x) {
            if (x <= low_th) {
                return x;
            } else if (x >= high_th) {
                const auto y = UseCurve ? paras[2] + paras[3] * x + paras[4] * x * x : paras[2] + paras[3] * x;
                return UseBound ? std::max(x - current_bound, y) : y;
            } else {
                const auto xx = x + paras[1];
                const auto y = x + paras[0] * xx * xx;
                return UseBound ? std::max(x - current_bound, y) : y;
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
            threshold.store(v);
            to_interpolate.store(true);
        }

        inline FloatType getThreshold() const { return threshold.load(); }

        inline void setRatio(FloatType v) {
            ratio.store(std::max(FloatType(1), v));
            to_interpolate.store(true);
        }

        inline FloatType getRatio() const { return ratio.load(); }

        inline void setKneeW(FloatType v) {
            knee_w.store(std::max(v, FloatType(0.01)));
            to_interpolate.store(true);
        }

        inline FloatType getKneeW() const { return knee_w.load(); }

        inline void setCurve(FloatType v) {
            curve.store(juce::jlimit(FloatType(-1), FloatType(1), v));
            to_interpolate.store(true);
        }

        inline FloatType getCurve() const { return curve.load(); }

        inline void setBound(FloatType v) {
            bound.store(v);
        }

        inline FloatType getBound() const { return bound.load(); }

        FloatType getReductionAtKnee() const { return reduction_at_knee; }

    private:
        LinearCurve<FloatType> linear_curve;
        DownCurve<FloatType> down_curve;
        UpCurve<FloatType> up_curve;
        std::atomic<FloatType> threshold{-18}, ratio{2};
        std::atomic<FloatType> knee_w{FloatType(0.25)}, curve{0};
        FloatType low_th{0}, high_th{0};
        std::atomic<FloatType> bound{60};
        FloatType current_bound{60};
        FloatType reduction_at_knee{FloatType(0.001)};
        std::array<FloatType, 5> paras;
        std::atomic<bool> to_interpolate{true};

        void interpolate() {
            const auto currentThreshold = threshold.load();
            const auto currentKneeW = knee_w.load();
            const auto currentRatio = ratio.load();
            if (UseBound) {
                current_bound = bound.load();
            }
            const auto currentCurve = curve.load();
            low_th = currentThreshold - currentKneeW;
            high_th = currentThreshold + currentKneeW;
            paras[0] = FloatType(1) / currentRatio - FloatType(1);
            paras[1] = -low_th;
            paras[0] *= FloatType(1) / (currentKneeW * FloatType(4));
            if (UseCurve) {
                if (currentCurve >= FloatType(0)) {
                    const auto alpha = FloatType(1) - currentCurve, beta = currentCurve;
                    linear_curve.setPara(currentThreshold, currentRatio, currentKneeW);
                    down_curve.setPara(currentThreshold, currentRatio, currentKneeW);
                    paras[2] = alpha * linear_curve.c + beta * down_curve.c;
                    paras[3] = alpha * linear_curve.b + beta * down_curve.b;
                    paras[4] = alpha * linear_curve.a + beta * down_curve.a;
                } else {
                    const auto alpha = FloatType(1) + currentCurve, beta = -currentCurve;
                    linear_curve.setPara(currentThreshold, currentRatio, currentKneeW);
                    up_curve.setPara(currentThreshold, currentRatio, currentKneeW);
                    paras[2] = alpha * linear_curve.c + beta * up_curve.c;
                    paras[3] = alpha * linear_curve.b + beta * up_curve.b;
                    paras[4] = alpha * linear_curve.a + beta * up_curve.a;
                }
            } else {
                linear_curve.setPara(currentThreshold, currentRatio, currentKneeW);
                paras[2] = linear_curve.c;
                paras[3] = linear_curve.b;
            }
            reduction_at_knee = std::max(FloatType(0.001), high_th - eval(high_th));
        }
    };
} // KneeComputer
