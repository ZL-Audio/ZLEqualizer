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
        constexpr double kLpA1 = 2.0 * kPhi0;
        constexpr double kLpA2 = kPhi0 * kPhi0;
        constexpr double kLpGScalar = 1.0 / (1.0 + kLpA1 + kLpA2);

        constexpr double kBpA1 = kPhi0 - 1.0;
        constexpr double kBpA2 = -kPhi0;
        constexpr double kBpGDenom = kPi * (1.0 + kPhi0);
        constexpr double kBpGMultiplier = kFourPiSq / kBpGDenom;

        constexpr double kDbToExp2 = 0.16609640474436813;
        constexpr double kDbToExp2Sqrt = kDbToExp2 * 0.5;

        struct PhiPair {
            double p1, p2, d;
        };

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

        inline PhiPair get_phi_sq(const double w2, const double zeta2) {
            const auto z_term = 2.0 * zeta2 - 1.0;
            const auto kappa = w2 * z_term + kSigmaSq;
            const auto v = std::sqrt(std::abs(w2 * w2 + 2.0 * kSigmaSq * w2 * z_term + kSigma4));
            const auto sq_term = std::sqrt(std::abs(2.0 * (v + kappa)));
            const auto pi_sq_term = kPi * sq_term;
            const auto d = kPiSq + pi_sq_term + v;
            const auto d_inv = 1.0 / d;
            return {(kTwoPiSq - 2.0 * v) * d_inv, (kPiSq - pi_sq_term + v) * d_inv, d};
        }

        inline PhiPair get_phi_notch_sq(const double w2) {
            const auto kappa = kSigmaSq - w2;
            const auto v = std::abs(w2 - kSigmaSq);
            const auto sq_term = std::sqrt(std::abs(2.0 * (v + kappa)));
            const auto pi_sq_term = kPi * sq_term;
            const auto d = kPiSq + pi_sq_term + v;
            const auto d_inv = 1.0 / d;
            return {(kTwoPiSq - 2.0 * v) * d_inv, (kPiSq - pi_sq_term + v) * d_inv, d};
        }

        inline PhiPair get_phi_peak(const double g, const double* cache) {
            const auto kappa = cache[0] * g + cache[1];
            const auto v = std::sqrt(std::abs(cache[2] * g + cache[3]));
            const auto sq_term = std::sqrt(std::abs(2.0 * (v + kappa)));
            const auto pi_sq_term = kPi * sq_term;
            const auto d = kPiSq + pi_sq_term + v;
            const auto d_inv = 1.0 / d;
            return {(kTwoPiSq - 2.0 * v) * d_inv, (kPiSq - pi_sq_term + v) * d_inv, d};
        }

        inline PhiPair get_phi_shelf(const double h, const double* cache) {
            const auto kappa = cache[0] * h + kSigmaSq;
            const auto v = std::sqrt(std::abs(cache[1] * h * h + cache[2] * h + kSigma4));
            const auto sq_term = std::sqrt(std::abs(2.0 * (v + kappa)));
            const auto pi_sq_term = kPi * sq_term;
            const auto d = kPiSq + pi_sq_term + v;
            const auto d_inv = 1.0 / d;
            return {(kTwoPiSq - 2.0 * v) * d_inv, (kPiSq - pi_sq_term + v) * d_inv, d};
        }

        inline std::array<double, 3> map1stOrderToSVF(const double a1, const double b0, const double b1) {
            const double c2 = 1.0 + a1;
            const double c0 = b0 / c2;
            const double c3 = (b0 + b1) / c2;
            return {c0, c2, c3};
        }

        inline std::array<double, 5> map2ndOrderToSVF(const double a1, const double a2,
                                                      const double b0, const double b1, const double b2) {
            const double c1 = (1.0 + a1 + a2) * 0.5;
            const double c2 = std::sqrt(std::max(0.0, (1.0 - a1 + a2) / (1.0 + a1 + a2)));
            const double c0 = b0 / c1;
            const double c3 = (b0 - b1 + b2) / (2.0 * c1 * c2);
            const double c4 = (b0 + b1 + b2) / (2.0 * c1);
            return {c0, c1, c2, c3, c4};
        }
    }

    std::array<double, 3> IvantsovSVFCoeff::get1LowPass(const double w0) {
        const auto w2 = get_wrapping_w2(w0);
        const auto beta = get_phi(w2);
        const auto factor = (1.0 / (1.0 + kPhi0)) * (1.0 + beta);
        return map1stOrderToSVF(beta, factor, factor * kPhi0);
    }

    std::array<double, 3> IvantsovSVFCoeff::get1HighPass(const double w0) {
        const auto w2 = get_wrapping_w2(w0);
        const auto beta = get_phi(w2);
        const auto factor = (std::sqrt(w2) / (2.0 * kPi)) * (1.0 + beta);
        return map1stOrderToSVF(beta, factor, -factor);
    }

    std::array<double, 3> IvantsovSVFCoeff::get1AllPass(const double w0) {
        const auto w2 = get_wrapping_w2(w0);
        const auto beta = get_phi(w2);
        return map1stOrderToSVF(beta, beta, 1.0);
    }

    std::array<double, 3> IvantsovSVFCoeff::get1LowShelf(const double w0, const double g) {
        const auto w2 = get_wrapping_w2(w0);
        const auto g_linear = std::exp2(g * kDbToExp2);
        const auto alpha = get_phi(w2 / g_linear);
        const auto beta = get_phi(w2 * g_linear);
        const auto factor = (g_linear / (1.0 + alpha)) * (1.0 + beta);
        return map1stOrderToSVF(beta, factor, factor * alpha);
    }

    std::array<double, 3> IvantsovSVFCoeff::get1HighShelf(const double w0, const double g) {
        const auto w2 = get_wrapping_w2(w0);
        const auto g_linear = std::exp2(g * kDbToExp2);
        const auto alpha = get_phi(w2 * g_linear);
        const auto beta = get_phi(w2 / g_linear);
        const auto factor = (1.0 / (1.0 + alpha)) * (1.0 + beta);
        return map1stOrderToSVF(beta, factor, factor * alpha);
    }

    std::array<double, 3> IvantsovSVFCoeff::get1TiltShelf(const double w0, const double g) {
        const auto w2 = get_wrapping_w2(w0);
        const auto g_half = std::exp2(g * kDbToExp2Sqrt);
        const auto g_linear = g_half * g_half;
        const auto alpha = get_phi(w2 * g_linear);
        const auto beta = get_phi(w2 / g_linear);
        const auto factor = ((1.0 / g_half) / (1.0 + alpha)) * (1.0 + beta);
        return map1stOrderToSVF(beta, factor, factor * alpha);
    }

    void IvantsovSVFCoeff::update1ShelfDynamicCache(const double w0, double* cache) {
        cache[0] = get_wrapping_w2(w0);
    }

    std::array<double, 3> IvantsovSVFCoeff::get1LowShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto w2 = cache[0];
        const auto g_linear = g_linear_sqrt * g_linear_sqrt;
        const auto alpha = get_phi(w2 / g_linear);
        const auto beta = get_phi(w2 * g_linear);
        const auto factor = (g_linear / (1.0 + alpha)) * (1.0 + beta);
        return map1stOrderToSVF(beta, factor, factor * alpha);
    }

    std::array<double, 3> IvantsovSVFCoeff::get1HighShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto w2 = cache[0];
        const auto g_linear = g_linear_sqrt * g_linear_sqrt;
        const auto alpha = get_phi(w2 * g_linear);
        const auto beta = get_phi(w2 / g_linear);
        const auto factor = (1.0 / (1.0 + alpha)) * (1.0 + beta);
        return map1stOrderToSVF(beta, factor, factor * alpha);
    }

    std::array<double, 3> IvantsovSVFCoeff::get1TiltShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto w2 = cache[0];
        const auto g_half = g_linear_sqrt;
        const auto g_linear = g_half * g_half;
        const auto alpha = get_phi(w2 * g_linear);
        const auto beta = get_phi(w2 / g_linear);
        const auto factor = ((1.0 / g_half) / (1.0 + alpha)) * (1.0 + beta);
        return map1stOrderToSVF(beta, factor, factor * alpha);
    }

    std::array<double, 5> IvantsovSVFCoeff::get2LowPass(const double w0, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto phi = get_phi_sq(w2, 1.0 / (4.0 * q * q));
        const auto factor = kLpGScalar * (1.0 + phi.p1 + phi.p2);
        return map2ndOrderToSVF(phi.p1, phi.p2, factor, factor * kLpA1, factor * kLpA2);
    }

    std::array<double, 5> IvantsovSVFCoeff::get2HighPass(const double w0, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto phi = get_phi_sq(w2, 1.0 / (4.0 * q * q));
        const auto factor = w2 / phi.d;
        return map2ndOrderToSVF(phi.p1, phi.p2, factor, factor * -2.0, factor);
    }

    std::array<double, 5> IvantsovSVFCoeff::get2BandPass(const double w0, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta = 1.0 / (2.0 * q);
        const auto phi = get_phi_sq(w2, zeta * zeta);
        const auto factor = (std::sqrt(w2) * zeta * kBpGMultiplier) / phi.d;
        return map2ndOrderToSVF(phi.p1, phi.p2, factor, factor * kBpA1, factor * kBpA2);
    }

    std::array<double, 5> IvantsovSVFCoeff::get2AllPass(const double w0, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto phi = get_phi_sq(w2, 1.0 / (4.0 * q * q));
        return map2ndOrderToSVF(phi.p1, phi.p2, phi.p2, phi.p1, 1.0);
    }

    std::array<double, 5> IvantsovSVFCoeff::get2Notch(const double w0, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto alpha = get_phi_notch_sq(w2);
        const auto beta = get_phi_sq(w2, 1.0 / (4.0 * q * q));
        const auto factor = alpha.d / beta.d;
        return map2ndOrderToSVF(beta.p1, beta.p2, factor, factor * alpha.p1, factor * alpha.p2);
    }

    std::array<double, 5> IvantsovSVFCoeff::get2Peak(const double w0, const double g, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta2 = 1.0 / (4.0 * q * q);
        const auto g_linear = std::exp2(g * kDbToExp2);
        const auto alpha = get_phi_sq(w2, zeta2 * g_linear);
        const auto beta = get_phi_sq(w2, zeta2 / g_linear);
        const auto factor = alpha.d / beta.d;
        return map2ndOrderToSVF(beta.p1, beta.p2, factor, factor * alpha.p1, factor * alpha.p2);
    }

    std::array<double, 5> IvantsovSVFCoeff::get2LowShelf(const double w0, const double g, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta2 = 1.0 / (4.0 * q * q);
        const auto g_half = std::exp2(g * kDbToExp2Sqrt);
        const auto alpha = get_phi_sq(w2 / g_half, zeta2);
        const auto beta = get_phi_sq(w2 * g_half, zeta2);
        double factor = (g_half * g_half) * (alpha.d / beta.d);
        return map2ndOrderToSVF(beta.p1, beta.p2, factor, factor * alpha.p1, factor * alpha.p2);
    }

    std::array<double, 5> IvantsovSVFCoeff::get2HighShelf(const double w0, const double g, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta2 = 1.0 / (4.0 * q * q);
        const auto g_half = std::exp2(g * kDbToExp2Sqrt);
        const auto alpha = get_phi_sq(w2 * g_half, zeta2);
        const auto beta = get_phi_sq(w2 / g_half, zeta2);
        const auto factor = alpha.d / beta.d;
        return map2ndOrderToSVF(beta.p1, beta.p2, factor, factor * alpha.p1, factor * alpha.p2);
    }

    std::array<double, 5> IvantsovSVFCoeff::get2TiltShelf(const double w0, const double g, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta2 = 1.0 / (4.0 * q * q);
        const auto g_half = std::exp2(g * kDbToExp2Sqrt);
        const auto inv_g_half = 1.0 / g_half;
        const auto alpha = get_phi_sq(w2 * g_half, zeta2);
        const auto beta = get_phi_sq(w2 * inv_g_half, zeta2);
        const auto factor = inv_g_half * (alpha.d / beta.d);
        return map2ndOrderToSVF(beta.p1, beta.p2, factor, factor * alpha.p1, factor * alpha.p2);
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
        const auto alpha = get_phi_peak(g_linear, cache);
        const auto beta = get_phi_peak(1.0 / g_linear, cache);
        const auto factor = alpha.d / beta.d;
        return map2ndOrderToSVF(beta.p1, beta.p2, factor, factor * alpha.p1, factor * alpha.p2);
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
        const auto alpha = get_phi_shelf(1.0 / g_linear_sqrt, cache);
        const auto beta = get_phi_shelf(g_linear_sqrt, cache);
        const auto factor = g_linear * (alpha.d / beta.d);
        return map2ndOrderToSVF(beta.p1, beta.p2, factor, factor * alpha.p1, factor * alpha.p2);
    }

    std::array<double, 5> IvantsovSVFCoeff::get2HighShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto alpha = get_phi_shelf(g_linear_sqrt, cache);
        const auto beta = get_phi_shelf(1.0 / g_linear_sqrt, cache);
        const auto factor = alpha.d / beta.d;
        return map2ndOrderToSVF(beta.p1, beta.p2, factor, factor * alpha.p1, factor * alpha.p2);
    }

    std::array<double, 5> IvantsovSVFCoeff::get2TiltShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto inv_g_linear_sqrt = 1.0 / g_linear_sqrt;
        const auto alpha = get_phi_shelf(g_linear_sqrt, cache);
        const auto beta = get_phi_shelf(inv_g_linear_sqrt, cache);
        const auto factor = inv_g_linear_sqrt * (alpha.d / beta.d);
        return map2ndOrderToSVF(beta.p1, beta.p2, factor, factor * alpha.p1, factor * alpha.p2);
    }
}
