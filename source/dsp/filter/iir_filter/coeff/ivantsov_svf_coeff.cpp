// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ivantsov_svf_coeff.hpp"
#include <cmath>
#include <algorithm>

namespace zldsp::filter {
    namespace {
        constexpr double kPi = 3.14159265358979323846;
        constexpr double kSigma = 2.00143;

        constexpr double kPiSq = kPi * kPi;
        constexpr double kTwoPiSq = 2.0 * kPiSq;
        constexpr double kFourPiSq = 4.0 * kPiSq;
        constexpr double kInvTwoPiSq = 1.0 / kTwoPiSq;
        constexpr double kInvPi = 1.0 / kPi;

        constexpr double kSigmaSq = kSigma * kSigma;
        constexpr double kSigma4 = kSigmaSq * kSigmaSq;

        constexpr double kSplineWA = 0.4996419767299294;
        constexpr double kSplineWB = 0.5919981009;
        constexpr double kSplineInvDelta = 10.8276522968691715;
        constexpr double kSplineA = 1.4079737347730696;
        constexpr double kSplineB = -2.5538880428234330;
        constexpr double kSplineC = -0.0064417990487326;
        constexpr double kSplineD = 4.0057345308722958;

        constexpr double kPhi0 = (kPi - kSigma) / (kPi + kSigma);

        constexpr double kInvTwoPi = 0.5 / kPi;
        constexpr double kLpC0 = 1.0 / (1.0 + kPhi0);

        constexpr double kLpA1 = 2.0 * kPhi0;
        constexpr double kLpA2 = kPhi0 * kPhi0;
        constexpr double kLpGScalar = 1.0 / (1.0 + kLpA1 + kLpA2);
        constexpr double kLpC3Numerator = kPi * kLpGScalar * (1.0 - kPhi0) * (1.0 - kPhi0);

        constexpr double kBpGDenom = kPi * (1.0 + kPhi0);
        constexpr double kBpGMultiplier = kFourPiSq / kBpGDenom;
        constexpr double kBpC0Numerator = kBpGMultiplier / (2.0 * kPiSq);
        constexpr double kBpC3Numerator = (kBpGMultiplier * (1.0 - kPhi0)) / (2.0 * kPi);

        constexpr double kDbToExp2 = 0.16609640474436813;
        constexpr double kDbToExp2Sqrt = kDbToExp2 * 0.5;

        inline double get_wrapping_w2(const double w0) {
            if (w0 <= kSplineWA) {
                const auto pi_fc_fs = kPi * w0;
                const auto tan_val = std::tan(pi_fc_fs);
                return kSigmaSq + kPiSq / (tan_val * tan_val);
            } else if (w0 >= kSplineWB) {
                return 1.0 / (w0 * w0);
            } else {
                const double t = (w0 - kSplineWA) * kSplineInvDelta;
                return t * (t * (kSplineA * t + kSplineB) + kSplineC) + kSplineD;
            }
        }

        inline double get_phi(const double w2) {
            const auto root = std::sqrt(w2 + kSigmaSq);
            return (kPi - root) / (kPi + root);
        }

        struct PhaseCore {
            double v;
            double d;
            double pi_sq_term;
        };

        inline PhaseCore get_svf_core(const double w2, const double zeta2) {
            const auto z_term = 2.0 * zeta2 - 1.0;
            const auto kappa = w2 * z_term + kSigmaSq;
            const auto v = std::sqrt(std::abs(w2 * w2 + 2.0 * kSigmaSq * w2 * z_term + kSigma4));
            const auto sq_term = std::sqrt(std::abs(2.0 * (v + kappa)));
            const auto pi_sq_term = kPi * sq_term;
            const auto d = kPiSq + pi_sq_term + v;
            return {v, d, pi_sq_term};
        }

        inline PhaseCore get_svf_notch_core(const double w2) {
            const auto kappa = kSigmaSq - w2;
            const auto v = std::abs(w2 - kSigmaSq);
            const auto sq_term = std::sqrt(std::abs(2.0 * (v + kappa)));
            const auto pi_sq_term = kPi * sq_term;
            const auto d = kPiSq + pi_sq_term + v;
            return {v, d, pi_sq_term};
        }

        inline PhaseCore get_svf_peak_core(const double g, const double* cache) {
            const auto kappa = cache[0] * g + cache[1];
            const auto v = std::sqrt(std::abs(cache[2] * g + cache[3]));
            const auto sq_term = std::sqrt(std::abs(2.0 * (v + kappa)));
            const auto pi_sq_term = kPi * sq_term;
            const auto d = kPiSq + pi_sq_term + v;
            return {v, d, pi_sq_term};
        }

        inline PhaseCore get_svf_shelf_core(const double h, const double* cache) {
            const auto kappa = cache[0] * h + kSigmaSq;
            const auto v = std::sqrt(std::abs(cache[1] * h * h + cache[2] * h + kSigma4));
            const auto sq_term = std::sqrt(std::abs(2.0 * (v + kappa)));
            const auto pi_sq_term = kPi * sq_term;
            const auto d = kPiSq + pi_sq_term + v;
            return {v, d, pi_sq_term};
        }
    }

    std::array<double, 3> IvantsovSVFCoeff::get1LowPass(const double w0) {
        const auto w2 = get_wrapping_w2(w0);
        const auto beta = get_phi(w2);

        return {kLpC0, 1.0 + beta, 1.0};
    }

    std::array<double, 3> IvantsovSVFCoeff::get1HighPass(const double w0) {
        const auto w2 = get_wrapping_w2(w0);
        const auto beta = get_phi(w2);

        const auto c0 = std::sqrt(w2) * kInvTwoPi;

        return {c0, 1.0 + beta, 0.0};
    }

    std::array<double, 3> IvantsovSVFCoeff::get1AllPass(const double w0) {
        const auto w2 = get_wrapping_w2(w0);
        const auto beta = get_phi(w2);

        const auto c1 = 1.0 + beta;

        return {beta / c1, c1, 1.0};
    }

    std::array<double, 3> IvantsovSVFCoeff::get1LowShelf(const double w0, const double g) {
        const auto w2 = get_wrapping_w2(w0);
        const auto g_linear = std::exp2(g * kDbToExp2);

        const auto alpha = get_phi(w2 / g_linear);
        const auto beta = get_phi(w2 * g_linear);

        const double c0 = g_linear / (1.0 + alpha);

        return {c0, 1.0 + beta, g_linear};
    }

    std::array<double, 3> IvantsovSVFCoeff::get1HighShelf(const double w0, const double g) {
        const auto w2 = get_wrapping_w2(w0);
        const auto g_linear = std::exp2(g * kDbToExp2);

        const auto alpha = get_phi(w2 * g_linear);
        const auto beta = get_phi(w2 / g_linear);

        const double c0 = 1.0 / (1.0 + alpha);

        return {c0, 1.0 + beta, 1.0};
    }

    std::array<double, 3> IvantsovSVFCoeff::get1TiltShelf(const double w0, const double g) {
        const auto w2 = get_wrapping_w2(w0);
        const auto g_half = std::exp2(g * kDbToExp2Sqrt);
        const auto g_linear = g_half * g_half;
        const auto inv_g_half = 1.0 / g_half;

        const auto alpha = get_phi(w2 * g_linear);
        const auto beta = get_phi(w2 / g_linear);

        const double c0 = inv_g_half / (1.0 + alpha);

        return {c0, 1.0 + beta, inv_g_half};
    }

    void IvantsovSVFCoeff::update1ShelfDynamicCache(const double w0, double* cache) {
        cache[0] = get_wrapping_w2(w0);
    }

    std::array<double, 3> IvantsovSVFCoeff::get1LowShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto w2 = cache[0];
        const auto g_linear = g_linear_sqrt * g_linear_sqrt;

        const auto alpha = get_phi(w2 / g_linear);
        const auto beta = get_phi(w2 * g_linear);

        const double c0 = g_linear / (1.0 + alpha);

        return {c0, 1.0 + beta, g_linear};
    }

    std::array<double, 3> IvantsovSVFCoeff::get1HighShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto w2 = cache[0];
        const auto g_linear = g_linear_sqrt * g_linear_sqrt;

        const auto alpha = get_phi(w2 * g_linear);
        const auto beta = get_phi(w2 / g_linear);

        const double c0 = 1.0 / (1.0 + alpha);

        return {c0, 1.0 + beta, 1.0};
    }

    std::array<double, 3> IvantsovSVFCoeff::get1TiltShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto w2 = cache[0];
        const auto g_half = g_linear_sqrt;
        const auto g_linear = g_half * g_half;
        const auto inv_g_half = 1.0 / g_half;

        const auto alpha = get_phi(w2 * g_linear);
        const auto beta = get_phi(w2 / g_linear);

        const double c0 = inv_g_half / (1.0 + alpha);

        return {c0, 1.0 + beta, inv_g_half};
    }

    std::array<double, 5> IvantsovSVFCoeff::get2LowPass(const double w0, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto core = get_svf_core(w2, 1.0 / (4.0 * q * q));

        const double v_sqrt = std::sqrt(core.v);

        const double c1 = kTwoPiSq / core.d;
        const double c2 = v_sqrt * kInvPi;
        const double c3 = kLpC3Numerator / v_sqrt;

        return {2.0 * kLpGScalar, c1, c2, c3, 1.0};
    }

    std::array<double, 5> IvantsovSVFCoeff::get2HighPass(const double w0, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto core = get_svf_core(w2, 1.0 / (4.0 * q * q));

        const double v_sqrt = std::sqrt(core.v);

        const double c1 = kTwoPiSq / core.d;
        const double c2 = v_sqrt * kInvPi;
        const double c0 = w2 * kInvTwoPiSq;
        const double c3 = w2 / (kPi * v_sqrt);

        return {c0, c1, c2, c3, 0.0};
    }

    std::array<double, 5> IvantsovSVFCoeff::get2BandPass(const double w0, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta = 1.0 / (2.0 * q);
        const auto core = get_svf_core(w2, zeta * zeta);

        const double w = std::sqrt(w2);
        const double v_sqrt = std::sqrt(core.v);

        const double c1 = kTwoPiSq / core.d;
        const double c2 = v_sqrt * kInvPi;
        const double c0 = w * zeta * kBpC0Numerator;
        const double c3 = w * zeta * kBpC3Numerator / v_sqrt;

        return {c0, c1, c2, c3, 0.0};
    }

    std::array<double, 5> IvantsovSVFCoeff::get2AllPass(const double w0, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto core = get_svf_core(w2, 1.0 / (4.0 * q * q));

        const double v_sqrt = std::sqrt(core.v);

        const double c1 = kTwoPiSq / core.d;
        const double c2 = v_sqrt * kInvPi;
        const double c0 = (kPiSq - core.pi_sq_term + core.v) * kInvTwoPiSq;

        return {c0, c1, c2, c2, 1.0};
    }

    std::array<double, 5> IvantsovSVFCoeff::get2Notch(const double w0, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto alpha = get_svf_notch_core(w2);
        const auto beta = get_svf_core(w2, 1.0 / (4.0 * q * q));

        const double beta_v_sqrt = std::sqrt(beta.v);

        const double c1 = kTwoPiSq / beta.d;
        const double c2 = beta_v_sqrt * kInvPi;
        const double c0 = alpha.d * kInvTwoPiSq;
        const double c3 = alpha.v / (kPi * beta_v_sqrt);

        return {c0, c1, c2, c3, 1.0};
    }

    std::array<double, 5> IvantsovSVFCoeff::get2Peak(const double w0, const double g, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta2 = 1.0 / (4.0 * q * q);
        const auto g_linear = std::exp2(g * kDbToExp2);

        const auto alpha = get_svf_core(w2, zeta2 * g_linear);
        const auto beta = get_svf_core(w2, zeta2 / g_linear);

        const double beta_v_sqrt = std::sqrt(beta.v);

        const double c1 = kTwoPiSq / beta.d;
        const double c2 = beta_v_sqrt * kInvPi;
        const double c0 = alpha.d * kInvTwoPiSq;
        const double c3 = alpha.v / (kPi * beta_v_sqrt);

        return {c0, c1, c2, c3, 1.0};
    }

    std::array<double, 5> IvantsovSVFCoeff::get2LowShelf(const double w0, const double g, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta2 = 1.0 / (4.0 * q * q);
        const auto g_half = std::exp2(g * kDbToExp2Sqrt);

        const auto alpha = get_svf_core(w2 / g_half, zeta2);
        const auto beta = get_svf_core(w2 * g_half, zeta2);

        const double g_linear = g_half * g_half;
        const double beta_v_sqrt = std::sqrt(beta.v);

        const double c1 = kTwoPiSq / beta.d;
        const double c2 = beta_v_sqrt * kInvPi;
        const double c0 = g_linear * alpha.d * kInvTwoPiSq;
        const double c3 = g_linear * alpha.v / (kPi * beta_v_sqrt);
        const double c4 = g_linear;

        return {c0, c1, c2, c3, c4};
    }

    std::array<double, 5> IvantsovSVFCoeff::get2HighShelf(const double w0, const double g, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta2 = 1.0 / (4.0 * q * q);
        const auto g_half = std::exp2(g * kDbToExp2Sqrt);

        const auto alpha = get_svf_core(w2 * g_half, zeta2);
        const auto beta = get_svf_core(w2 / g_half, zeta2);

        const double beta_v_sqrt = std::sqrt(beta.v);

        const double c1 = kTwoPiSq / beta.d;
        const double c2 = beta_v_sqrt * kInvPi;
        const double c0 = alpha.d * kInvTwoPiSq;
        const double c3 = alpha.v / (kPi * beta_v_sqrt);

        return {c0, c1, c2, c3, 1.0};
    }

    std::array<double, 5> IvantsovSVFCoeff::get2TiltShelf(const double w0, const double g, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta2 = 1.0 / (4.0 * q * q);
        const auto g_half = std::exp2(g * kDbToExp2Sqrt);
        const auto inv_g_half = 1.0 / g_half;

        const auto alpha = get_svf_core(w2 * g_half, zeta2);
        const auto beta = get_svf_core(w2 * inv_g_half, zeta2);

        const double beta_v_sqrt = std::sqrt(beta.v);

        const double c1 = kTwoPiSq / beta.d;
        const double c2 = beta_v_sqrt * kInvPi;
        const double c0 = inv_g_half * alpha.d * kInvTwoPiSq;
        const double c3 = inv_g_half * alpha.v / (kPi * beta_v_sqrt);
        const double c4 = inv_g_half;

        return {c0, c1, c2, c3, c4};
    }

    void IvantsovSVFCoeff::update2PeakDynamicCache(const double w0, const double q, double* cache) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta2 = 1.0 / (4.0 * q * q);
        cache[0] = 2.0 * w2 * zeta2;
        cache[1] = kSigmaSq - w2;
        cache[2] = 2.0 * kSigmaSq * cache[0];
        cache[3] = cache[1] * cache[1];
    }

    std::array<double, 5> IvantsovSVFCoeff::get2PeakWithCache(const double g_linear_sqrt, const double* cache) {
        const auto g_linear = g_linear_sqrt * g_linear_sqrt;

        const auto alpha = get_svf_peak_core(g_linear, cache);
        const auto beta = get_svf_peak_core(1.0 / g_linear, cache);
        const double beta_v_sqrt = std::sqrt(beta.v);

        const double c1 = kTwoPiSq / beta.d;
        const double c2 = beta_v_sqrt * kInvPi;
        const double c0 = alpha.d * kInvTwoPiSq;
        const double c3 = alpha.v / (kPi * beta_v_sqrt);

        return {c0, c1, c2, c3, 1.0};
    }

    void IvantsovSVFCoeff::update2ShelfDynamicCache(const double w0, const double q, double* cache) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta2 = 1.0 / (4.0 * q * q);
        cache[0] = w2 * (2.0 * zeta2 - 1.0);
        cache[1] = w2 * w2;
        cache[2] = 2.0 * kSigmaSq * cache[0];
    }

    std::array<double, 5> IvantsovSVFCoeff::get2LowShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto g_linear = g_linear_sqrt * g_linear_sqrt;

        const auto alpha = get_svf_shelf_core(1.0 / g_linear_sqrt, cache);
        const auto beta = get_svf_shelf_core(g_linear_sqrt, cache);
        const double beta_v_sqrt = std::sqrt(beta.v);

        const double c1 = kTwoPiSq / beta.d;
        const double c2 = beta_v_sqrt * kInvPi;
        const double c0 = g_linear * alpha.d * kInvTwoPiSq;
        const double c3 = g_linear * alpha.v / (kPi * beta_v_sqrt);
        const double c4 = g_linear;

        return {c0, c1, c2, c3, c4};
    }

    std::array<double, 5> IvantsovSVFCoeff::get2HighShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto alpha = get_svf_shelf_core(g_linear_sqrt, cache);
        const auto beta = get_svf_shelf_core(1.0 / g_linear_sqrt, cache);
        const double beta_v_sqrt = std::sqrt(beta.v);

        const double c1 = kTwoPiSq / beta.d;
        const double c2 = beta_v_sqrt * kInvPi;
        const double c0 = alpha.d * kInvTwoPiSq;
        const double c3 = alpha.v / (kPi * beta_v_sqrt);

        return {c0, c1, c2, c3, 1.0};
    }

    std::array<double, 5> IvantsovSVFCoeff::get2TiltShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto inv_g_linear_sqrt = 1.0 / g_linear_sqrt;

        const auto alpha = get_svf_shelf_core(g_linear_sqrt, cache);
        const auto beta = get_svf_shelf_core(inv_g_linear_sqrt, cache);
        const double beta_v_sqrt = std::sqrt(beta.v);

        const double c1 = kTwoPiSq / beta.d;
        const double c2 = beta_v_sqrt * kInvPi;
        const double c0 = inv_g_linear_sqrt * alpha.d * kInvTwoPiSq;
        const double c3 = inv_g_linear_sqrt * alpha.v / (kPi * beta_v_sqrt);
        const double c4 = inv_g_linear_sqrt;

        return {c0, c1, c2, c3, c4};
    }
}
