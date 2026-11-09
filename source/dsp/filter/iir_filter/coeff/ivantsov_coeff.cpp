// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ivantsov_coeff.hpp"
#include "ivantsov_common.hpp"
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

        inline std::array<double, 5> make_coefficients(const PhiPair& alpha, const PhiPair& beta,
                                                       const double gain = 1.0) {
            const auto inv_d = 1.0 / beta.d;
            const auto scale = gain * inv_d;
            return {beta.p1 * inv_d, beta.p2 * inv_d,
                    alpha.d * scale, alpha.p1 * scale, alpha.p2 * scale};
        }

        inline std::array<double, 3> get_shelf(const double alpha_w2, const double beta_w2,
                                               const double gain) {
            const auto alpha_root = std::sqrt(alpha_w2 + kSigmaSq);
            const auto beta_root = std::sqrt(beta_w2 + kSigmaSq);
            const auto inv_d = 1.0 / (kPi + beta_root);
            const auto scale = gain * inv_d;
            return {(kPi - beta_root) * inv_d,
                    (kPi + alpha_root) * scale, (kPi - alpha_root) * scale};
        }

        inline double get_wrapping_w2(const double w0) {
            return ivantsov::getWrappingW2(w0);
        }

        inline double get_phi(const double w2) {
            const auto root = std::sqrt(w2 + kSigmaSq);
            return (kPi - root) / (kPi + root);
        }

        inline PhiPair get_phi_sq(const double w2, const double zeta2) {
            const auto core = ivantsov::getCore(w2, zeta2);
            return {kTwoPiSq - 2.0 * core.v, kPiSq - core.pi_sq_term + core.v, core.d};
        }

        inline PhiPair get_phi_notch_sq(const double w2) {
            const auto kappa = kSigmaSq - w2;
            const auto v = std::abs(w2 - kSigmaSq);

            const auto sq_term = std::sqrt(std::abs(2.0 * (v + kappa)));
            const auto pi_sq_term = kPi * sq_term;

            const auto d = kPiSq + pi_sq_term + v;
            return {kTwoPiSq - 2.0 * v, kPiSq - pi_sq_term + v, d};
        }

        inline PhiPair get_phi_peak(const double g, const double* cache) {
            const auto kappa = cache[0] * g + cache[1];
            const auto v = std::sqrt(std::abs(cache[2] * g + cache[3]));

            const auto sq_term = std::sqrt(std::abs(2.0 * (v + kappa)));
            const auto pi_sq_term = kPi * sq_term;

            const auto d = kPiSq + pi_sq_term + v;
            return {kTwoPiSq - 2.0 * v, kPiSq - pi_sq_term + v, d};
        }

        inline PhiPair get_phi_shelf(const double h, const double* cache) {
            const auto kappa = cache[0] * h + kSigmaSq;
            const auto v = std::sqrt(std::abs(cache[1] * h * h + cache[2] * h + kSigma4));

            const auto sq_term = std::sqrt(std::abs(2.0 * (v + kappa)));
            const auto pi_sq_term = kPi * sq_term;

            const auto d = kPiSq + pi_sq_term + v;
            return {kTwoPiSq - 2.0 * v, kPiSq - pi_sq_term + v, d};
        }
    }

    std::array<double, 3> IvantsovCoeff::get1LowPass(const double w0) {
        return get1PassFromW2(get_wrapping_w2(w0), false);
    }

    std::array<double, 3> IvantsovCoeff::get1HighPass(const double w0) {
        return get1PassFromW2(get_wrapping_w2(w0), true);
    }

    std::array<double, 3> IvantsovCoeff::get1AllPass(const double w0) {
        const auto w2 = get_wrapping_w2(w0);
        const auto beta = get_phi(w2);

        return {beta, beta, 1.0};
    }

    std::array<double, 3> IvantsovCoeff::get1LowShelf(const double w0, const double g) {
        const auto w2 = get_wrapping_w2(w0);
        const auto g_linear = std::exp2(g * kDbToExp2);

        return get_shelf(w2 / g_linear, w2 * g_linear, g_linear);
    }

    std::array<double, 3> IvantsovCoeff::get1HighShelf(const double w0, const double g) {
        const auto w2 = get_wrapping_w2(w0);
        const auto g_linear = std::exp2(g * kDbToExp2);

        return get_shelf(w2 * g_linear, w2 / g_linear, 1.0);
    }

    std::array<double, 3> IvantsovCoeff::get1TiltShelf(const double w0, const double g) {
        const auto w2 = get_wrapping_w2(w0);
        const auto g_half = std::exp2(g * kDbToExp2Sqrt);
        const auto g_linear = g_half * g_half;

        return get_shelf(w2 * g_linear, w2 / g_linear, 1.0 / g_half);
    }

    void IvantsovCoeff::update1ShelfDynamicCache(const double w0, double* cache) {
        cache[0] = get_wrapping_w2(w0);
    }

    std::array<double, 3> IvantsovCoeff::get1LowShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto w2 = cache[0];
        const auto g_linear = g_linear_sqrt * g_linear_sqrt;

        return get_shelf(w2 / g_linear, w2 * g_linear, g_linear);
    }

    std::array<double, 3> IvantsovCoeff::get1HighShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto w2 = cache[0];
        const auto g_linear = g_linear_sqrt * g_linear_sqrt;

        return get_shelf(w2 * g_linear, w2 / g_linear, 1.0);
    }

    std::array<double, 3> IvantsovCoeff::get1TiltShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto w2 = cache[0];
        const auto g_half = g_linear_sqrt;
        const auto g_linear = g_half * g_half;

        return get_shelf(w2 * g_linear, w2 / g_linear, 1.0 / g_half);
    }

    std::array<double, 5> IvantsovCoeff::get2LowPass(const double w0, const double q) {
        return get2PassFromW2(get_wrapping_w2(w0), false, q);
    }

    std::array<double, 5> IvantsovCoeff::get2HighPass(const double w0, const double q) {
        return get2PassFromW2(get_wrapping_w2(w0), true, q);
    }

    std::array<double, 5> IvantsovCoeff::get2BandPass(const double w0, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta = 1.0 / (2.0 * q);
        const auto phi = get_phi_sq(w2, zeta * zeta);
        const auto inv_d = 1.0 / phi.d;
        const auto factor = (std::sqrt(w2) * zeta * kBpGMultiplier) * inv_d;

        return {phi.p1 * inv_d, phi.p2 * inv_d, factor, factor * kBpA1, factor * kBpA2};
    }

    std::array<double, 5> IvantsovCoeff::get2AllPass(const double w0, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto phi = get_phi_sq(w2, 1.0 / (4.0 * q * q));
        const auto inv_d = 1.0 / phi.d;
        const auto p1 = phi.p1 * inv_d;
        const auto p2 = phi.p2 * inv_d;

        return {p1, p2, p2, p1, 1.0};
    }

    std::array<double, 5> IvantsovCoeff::get2Notch(const double w0, const double q) {
        const auto w2 = get_wrapping_w2(w0);
        const auto alpha = get_phi_notch_sq(w2);
        const auto beta = get_phi_sq(w2, 1.0 / (4.0 * q * q));

        return make_coefficients(alpha, beta);
    }

    std::array<double, 5> IvantsovCoeff::get2Peak(const double w0, const double g, const double q) {
        return getPeakFromW2(get_wrapping_w2(w0), g, q);
    }

    IvantsovCoeff::ShelfContext IvantsovCoeff::get2ShelfContext(const double w0, const double g) {
        return {get_wrapping_w2(w0), std::exp2(g * kDbToExp2Sqrt)};
    }

    std::array<double, 5> IvantsovCoeff::get2LowShelf(const double w0, const double g, const double q) {
        return get2LowShelf(get2ShelfContext(w0, g), q);
    }

    std::array<double, 5> IvantsovCoeff::get2LowShelf(const ShelfContext& context, const double q) {
        const auto w2 = context.w2;
        const auto zeta2 = 1.0 / (4.0 * q * q);
        const auto g_half = context.gain_sqrt;
        const auto alpha = get_phi_sq(w2 / g_half, zeta2);
        const auto beta = get_phi_sq(w2 * g_half, zeta2);

        return make_coefficients(alpha, beta, g_half * g_half);
    }

    std::array<double, 5> IvantsovCoeff::get2HighShelf(const double w0, const double g, const double q) {
        return get2HighShelf(get2ShelfContext(w0, g), q);
    }

    std::array<double, 5> IvantsovCoeff::get2HighShelf(const ShelfContext& context, const double q) {
        const auto w2 = context.w2;
        const auto zeta2 = 1.0 / (4.0 * q * q);
        const auto g_half = context.gain_sqrt;
        const auto alpha = get_phi_sq(w2 * g_half, zeta2);
        const auto beta = get_phi_sq(w2 / g_half, zeta2);

        return make_coefficients(alpha, beta);
    }

    std::array<double, 5> IvantsovCoeff::get2TiltShelf(const double w0, const double g, const double q) {
        return get2TiltShelf(get2ShelfContext(w0, g), q);
    }

    std::array<double, 5> IvantsovCoeff::get2TiltShelf(const ShelfContext& context, const double q) {
        const auto w2 = context.w2;
        const auto zeta2 = 1.0 / (4.0 * q * q);
        const auto g_half = context.gain_sqrt;
        const auto inv_g_half = 1.0 / g_half;
        const auto alpha = get_phi_sq(w2 * g_half, zeta2);
        const auto beta = get_phi_sq(w2 * inv_g_half, zeta2);

        return make_coefficients(alpha, beta, inv_g_half);
    }

    void IvantsovCoeff::update2PeakDynamicCache(const double w0, const double q, double* cache) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta2 = 1.0 / (4.0 * q * q);
        cache[0] = 2.0 * w2 * zeta2;
        cache[1] = kSigmaSq - w2;
        cache[2] = 2.0 * kSigmaSq * cache[0];
        cache[3] = cache[1] * cache[1];
    }

    std::array<double, 5> IvantsovCoeff::get2PeakWithCache(const double g_linear_sqrt, const double* cache) {
        const auto g_linear = g_linear_sqrt * g_linear_sqrt;
        const auto alpha = get_phi_peak(g_linear, cache);
        const auto beta = get_phi_peak(1.0 / g_linear, cache);

        return make_coefficients(alpha, beta);
    }

    void IvantsovCoeff::update2ShelfDynamicCache(const double w0, const double q, double* cache) {
        const auto w2 = get_wrapping_w2(w0);
        const auto zeta2 = 1.0 / (4.0 * q * q);
        cache[0] = w2 * (2.0 * zeta2 - 1.0);
        cache[1] = w2 * w2;
        cache[2] = 2.0 * kSigmaSq * cache[0];
    }

    std::array<double, 5> IvantsovCoeff::get2LowShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto g_linear = g_linear_sqrt * g_linear_sqrt;
        const auto alpha = get_phi_shelf(1.0 / g_linear_sqrt, cache);
        const auto beta = get_phi_shelf(g_linear_sqrt, cache);

        return make_coefficients(alpha, beta, g_linear);
    }

    std::array<double, 5> IvantsovCoeff::get2HighShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto alpha = get_phi_shelf(g_linear_sqrt, cache);
        const auto beta = get_phi_shelf(1.0 / g_linear_sqrt, cache);

        return make_coefficients(alpha, beta);
    }

    std::array<double, 5> IvantsovCoeff::get2TiltShelfWithCache(const double g_linear_sqrt, const double* cache) {
        const auto inv_g_linear_sqrt = 1.0 / g_linear_sqrt;
        const auto alpha = get_phi_shelf(g_linear_sqrt, cache);
        const auto beta = get_phi_shelf(inv_g_linear_sqrt, cache);

        return make_coefficients(alpha, beta, inv_g_linear_sqrt);
    }

    std::array<double, 3> IvantsovCoeff::get1PassFromW2(const double w2, const bool hp) {
        if (hp) {
            const auto beta = get_phi(w2);
            const auto factor = (std::sqrt(w2) / (2.0 * kPi)) * (1.0 + beta);

            return {beta, factor, -factor};
        }
        const auto beta = get_phi(w2);
        const auto factor = (1.0 / (1.0 + kPhi0)) * (1.0 + beta);

        return {beta, factor, factor * kPhi0};
    }

    std::array<double, 5> IvantsovCoeff::get2PassFromW2(const double w2, const bool hp, const double q) {
        if (hp) {
            const auto phi = get_phi_sq(w2, 1.0 / (4.0 * q * q));
            const auto inv_d = 1.0 / phi.d;
            const auto factor = w2 * inv_d;

            return {phi.p1 * inv_d, phi.p2 * inv_d, factor, factor * -2.0, factor};
        }
        const auto phi = get_phi_sq(w2, 1.0 / (4.0 * q * q));
        const auto inv_d = 1.0 / phi.d;
        const auto factor = (kLpGScalar * kFourPiSq) * inv_d;

        return {phi.p1 * inv_d, phi.p2 * inv_d, factor, factor * kLpA1, factor * kLpA2};
    }

    std::array<double, 5> IvantsovCoeff::getPeakFromW2(const double w2, const double g, const double q) {
        const auto zeta2 = 1.0 / (4.0 * q * q);
        const auto g_linear = std::exp2(g * kDbToExp2);
        const auto alpha = get_phi_sq(w2, zeta2 * g_linear);
        const auto beta = get_phi_sq(w2, zeta2 / g_linear);

        return make_coefficients(alpha, beta);
    }
}
