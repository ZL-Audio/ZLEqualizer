// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

// Reference:
// Matched One-Pole Digital Shelving Filters, Martin Vicanek
// Matched Second Order Digital Filters, Martin Vicanek

#include "martin_coeff.hpp"
#include "../../helpers.hpp"

namespace zldsp::filter {
    std::array<double, 4> MartinCoeff::get1LowPass(const double w0) {
        const auto fc = w0 / kPi;
        const auto fm = 0.5 * std::sqrt(fc * fc + 1);
        const auto phim = 1 - std::cos(kPi * fm);

        const double a1 = -std::exp(-w0);

        const auto alpha = -2 * a1 / std::pow(1 + a1, 2);
        const auto k = (fc * fc) / (fc * fc + fm * fm);
        const auto beta = k * alpha + (k - 1) / phim;
        const auto b_temp = -beta / (1 + beta + std::sqrt(1 + 2 * beta));

        const auto b0 = (1.0 + a1) / (1.0 + b_temp);
        const auto b1 = b_temp * b0;
        return {1.0, a1, b0, b1};
    }

    std::array<double, 4> MartinCoeff::get1HighPass(const double w0) {
        const auto wm = w0 * 0.5;
        const std::array<double, 2> phim = {1 - std::pow(std::sin(wm / 2), 2), std::pow(std::sin(wm / 2), 2)};

        const auto a1 = -std::exp(-w0);

        const std::array<double, 2> A = {std::pow(1.0 + a1, 2), std::pow(1.0 - a1, 2)};
        const auto B1 = (wm * wm) / (wm * wm + w0 * w0) * (A[0] * phim[0] + A[1] * phim[1]) / phim[1];
        const auto b0 = 0.5 * std::sqrt(B1);

        return {1.0, a1, b0, -b0};
    }

    std::array<double, 4> MartinCoeff::get1TiltShelf(const double w0, const double g) {
        const auto fc = w0 / kPi;
        const auto fm = fc * 0.75;
        const auto phim = 1 - std::cos(kPi * fm);
        const auto alpha = 2 / kPi2 * (1 / std::pow(fm, 2) + 1 / g / std::pow(fc, 2)) - 1 / phim;
        const auto beta = 2 / kPi2 * (1 / std::pow(fm, 2) + g / std::pow(fc, 2)) - 1 / phim;

        const auto a1 = -alpha / (1 + alpha + std::sqrt(1 + 2 * alpha));

        const auto b_temp = -beta / (1 + beta + std::sqrt(1 + 2 * beta));

        const auto b0 = (1 + a1) / (1 + b_temp) / std::sqrt(g);
        const auto b1 = b_temp * b0;
        return {1.0, a1, b0, b1};
    }

    std::array<double, 4> MartinCoeff::get1LowShelf(const double w0, const double g) {
        const auto ab = get1TiltShelf(w0, 1.0 / g);
        const auto A = std::sqrt(g);
        return {1.0, ab[1], ab[2] * A, ab[3] * A};
    }

    std::array<double, 4> MartinCoeff::get1HighShelf(const double w0, const double g) {
        const auto ab = get1TiltShelf(w0, g);
        const auto A = std::sqrt(g);
        return {1.0, ab[1], ab[2] * A, ab[3] * A};
    }

    std::array<double, 6> MartinCoeff::get2LowPass(const double w0, const double q) {
        const auto a = solve_a(w0, 0.5 / q, 1);
        const auto A = get_AB(a);
        std::array<double, 3> ws{};
        if (w0 > kPi / 32) {
            ws = {0, 0.5 * w0, w0};
        } else {
            ws = {kPi, w0, 0.5 * (kPi + w0)};
        }
        std::array<std::array<double, 3>, 3> phi{};
        std::array<double, 3> res{};
        for (size_t i = 0; i < 3; ++i) {
            phi[i] = get_phi(ws[i]);
            res[i] = AnalogFunc::get2LowPassMagnitude2(w0, q, ws[i]) * dotProduct(phi[i], A);
        }
        const auto B = linear_solve(phi, res);
        const auto b = get_ab(B);
        return {a[0], a[1], a[2], b[0], b[1], b[2]};
    }

    std::array<double, 6> MartinCoeff::get2HighPass(const double w0, const double q) {
        const auto a = solve_a(w0, 0.5 / q, 1);
        const auto A = get_AB(a);
        const auto phi0 = get_phi(w0);

        std::array<double, 3> b{};
        b[0] = q * std::sqrt(dotProduct(A, phi0)) / 4 / phi0[1];
        b[1] = -2 * b[0];
        b[2] = b[0];
        return {a[0], a[1], a[2], b[0], b[1], b[2]};
    }

    std::array<double, 6> MartinCoeff::get2BandPass(const double w0, double q) {
        q = std::max(q, 0.025);
        const auto a = solve_a(w0, 0.5 / q);
        const auto A = get_AB(a);

        if (w0 > kPi / 32) {
            const auto phi0 = get_phi(w0);
            const auto R1 = dotProduct(phi0, A);
            const auto R2 = dotProduct({-1, 1, 4 * (phi0[0] - phi0[1])}, A);

            std::array<double, 3> B{};
            B[0] = 0.0;
            B[2] = (R1 - R2 * phi0[1]) / 4 / std::pow(phi0[1], 2);
            B[1] = R2 + 4 * (phi0[1] - phi0[0]) * B[2];

            const auto b = get_ab(B);
            return {a[0], a[1], a[2], b[0], b[1], b[2]};
        } else {
            const auto _w = getBandwidth(w0, q);
            const auto w1 = _w[0], w2 = _w[1];
            std::array<double, 3> ws{0, w0, w0 > kPiHalf ? w1 : w2};
            const auto _ws = ws;
            std::array<double, 3> B{-1, -1, -1};
            size_t trial = 0;
            while (!check_AB(B) && trial < 20) {
                trial += 1;
                std::array<std::array<double, 3>, 3> phi{};
                std::array<double, 3> res{};
                for (size_t i = 0; i < 3; ++i) {
                    phi[i] = get_phi(ws[i]);
                    res[i] = AnalogFunc::get2BandPassMagnitude2(w0, q, ws[i]) * dotProduct(phi[i], A);
                }
                B = linear_solve(phi, res);
                ws[2] = w0 > kPiHalf ? 0.9 * ws[2] : 0.9 * ws[2] + 0.1 * kPi;
            }
            if (trial == 20) {
                ws = _ws;
                std::array<std::array<double, 3>, 3> phi{};
                std::array<double, 3> res{};
                for (size_t i = 0; i < 3; ++i) {
                    phi[i] = get_phi(ws[i]);
                    res[i] = AnalogFunc::get2BandPassMagnitude2(w0, q, ws[i]) * dotProduct(phi[i], A);
                }
                B = linear_solve(phi, res);
                ws[2] = w0 > kPiHalf ? 0.9 * ws[2] : 0.9 * ws[2] + 0.1 * kPi;
            }
            const auto b = get_ab(B);
            return {a[0], a[1], a[2], b[0], b[1], b[2]};
        }
    }

    std::array<double, 6> MartinCoeff::get2Notch(const double w0, const double q) {
        std::array<double, 3> b{};
        if (w0 < kPi) {
            b = {1, -2 * std::cos(w0), 1};
        } else {
            b = {1, -2 * std::sinh(w0), 1};
        }
        const auto B = get_AB(b);

        const auto _w = getBandwidth(w0, q);
        const auto w1 = _w[0], w2 = _w[1];
        const std::array<double, 3> ws{0, w1, w2 < kPi ? w2 : 0.5 * (w0 + w1)};

        std::array<std::array<double, 3>, 3> phi{};
        std::array<double, 3> res{};
        for (size_t i = 0; i < 3; ++i) {
            phi[i] = get_phi(ws[i]);
            res[i] = dotProduct(phi[i], B) / AnalogFunc::get2NotchMagnitude2(w0, q, ws[i]);
        }

        const auto A = linear_solve(phi, res);
        const auto a = get_ab(A);
        return {a[0], a[1], a[2], b[0], b[1], b[2]};
    }

    std::array<double, 6> MartinCoeff::get2Peak(double w0, double g, double q) {
        const auto a = solve_a(w0, 0.5 / std::sqrt(g) / q);
        const auto A = get_AB(a);
        const auto phi0 = get_phi(w0);

        const auto R1 = dotProduct(A, phi0) * std::pow(g, 2);
        const auto R2 = (-A[0] + A[1] + 4 * (phi0[0] - phi0[1]) * A[2]) * std::pow(g, 2);

        std::array<double, 3> B{A[0], 0, 0};
        B[2] = (R1 - R2 * phi0[1] - B[0]) / (4 * std::pow(phi0[1], 2));
        B[1] = R2 + B[0] + 4 * (phi0[1] - phi0[0]) * B[2];
        const auto b = get_ab(B);

        return {a[0], a[1], a[2], b[0], b[1], b[2]};
    }

    std::array<double, 6> MartinCoeff::get2TiltShelf(const double w0, double g, const double q) {
        const bool reverse_ab = (g > 1);
        if (g > 1) {
            g = 1 / g;
        }
        const auto g_sqrt = std::sqrt(g);
        const auto a = solve_a(w0, std::sqrt(g_sqrt) / 2 / q, std::sqrt(g_sqrt));
        const auto A = get_AB(a);

        const auto c2 = g_sqrt * (-1 + 2 * q * q);
        const auto c0 = c2 * std::pow(w0, 4);
        const auto c1 = -2 * (1 + g) * std::pow(q * w0, 2);
        auto delta = c1 * c1 - 4 * c0 * c2;
        std::array<double, 3> ws{};
        if (delta <= 0) {
            ws = {0, w0 / 2, w0};
        } else {
            delta = std::sqrt(delta);
            const auto sol1 = (-c1 + delta) / 2 / c2;
            const auto sol2 = (-c1 - delta) / 2 / c2;
            if (sol1 < 0 || sol2 < 0) {
                ws = {0, w0 / 2, w0};
            } else {
                const auto w1 = std::sqrt((-c1 + delta) / 2 / c2);
                const auto w2 = std::sqrt((-c1 - delta) / 2 / c2);
                if (w1 < kPi || w2 < kPi) {
                    ws = {0, std::min(w1, w2), std::min(std::max(w1, w2), kPi)};
                } else {
                    ws = {0, kPiHalf, kPi};
                }
            }
        }
        std::array<double, 3> B{-1, -1, -1};
        size_t trial = 0;
        const auto _ws = ws;
        while (!check_AB(B) && trial < 20) {
            trial += 1;
            std::array<std::array<double, 3>, 3> phi{};
            std::array<double, 3> res{};
            for (size_t i = 0; i < 3; ++i) {
                phi[i] = get_phi(ws[i]);
                res[i] = AnalogFunc::get2TiltShelfMagnitude2(w0, g, q, ws[i]) * dotProduct(phi[i], A);
            }
            B = linear_solve(phi, res);
            ws[2] = 0.5 * (ws[2] + kPi);
        }
        if (trial == 20) {
            ws = _ws;
            std::array<std::array<double, 3>, 3> phi{};
            std::array<double, 3> res{};
            for (size_t i = 0; i < 3; ++i) {
                phi[i] = get_phi(ws[i]);
                res[i] = AnalogFunc::get2TiltShelfMagnitude2(w0, g, q, ws[i]) * dotProduct(phi[i], A);
            }
            B = linear_solve(phi, res);
            ws[2] = w0 > kPiHalf ? 0.9 * ws[2] : 0.9 * ws[2] + 0.1 * kPi;
        }
        const auto b = get_ab(B);
        if (reverse_ab) {
            return {b[0], b[1], b[2], a[0], a[1], a[2]};
        } else {
            return {a[0], a[1], a[2], b[0], b[1], b[2]};
        }
    }

    std::array<double, 6> MartinCoeff::get2LowShelf(const double w0, const double g, const double q) {
        const auto ab = get2TiltShelf(w0, 1 / g, q);
        const auto A = std::sqrt(g);
        return {ab[0], ab[1], ab[2], ab[3] * A, ab[4] * A, ab[5] * A};
    }

    std::array<double, 6> MartinCoeff::get2HighShelf(const double w0, const double g, const double q) {
        const auto ab = get2TiltShelf(w0, g, q);
        const auto A = std::sqrt(g);
        return {ab[0], ab[1], ab[2], ab[3] * A, ab[4] * A, ab[5] * A};
    }

    std::array<double, 3> MartinCoeff::solve_a(const double w0, const double b, const double c) {
        std::array<double, 3> a{};
        a[0] = 1.0;
        if (b <= c) {
            a[1] = -2 * std::exp(-b * w0) * std::cos(std::sqrt(c * c - b * b) * w0);
        } else {
            a[1] = -2 * std::exp(-b * w0) * std::cosh(std::sqrt(b * b - c * c) * w0);
        }
        a[2] = std::exp(-2 * b * w0);
        return a;
    }

    std::array<double, 3> MartinCoeff::get_AB(const std::array<double, 3> &a) {
        std::array<double, 3> A{};
        A[0] = std::pow(a[0] + a[1] + a[2], 2);
        A[1] = std::pow(a[0] - a[1] + a[2], 2);
        A[2] = -4 * a[2];
        return A;
    }

    bool MartinCoeff::check_AB(const std::array<double, 3> &A) {
        return A[0] > 0 && A[1] > 0 && std::pow(0.5 * (std::sqrt(A[0]) + std::sqrt(A[1])), 2) + A[2] > 0;
    }

    std::array<double, 3> MartinCoeff::get_ab(const std::array<double, 3> &A) {
        std::array<double, 3> a{};
        const auto A0 = std::sqrt(std::max(A[0], 0.0));
        const auto A1 = std::sqrt(std::max(A[1], 0.0));
        auto W = 0.5 * (A0 + A1);
        auto temp = std::max(W * W + A[2], 0.0);
        a[0] = 0.5 * (W + std::sqrt(temp));
        a[1] = 0.5 * (A0 - A1);
        a[2] = -A[2] / 4 / a[0];
        return a;
    }

    std::array<double, 3> MartinCoeff::get_phi(const double w) {
        std::array<double, 3> phi{};
        phi[0] = 1 - std::pow(std::sin(w / 2), 2);
        phi[1] = 1 - phi[0];
        phi[2] = 4 * phi[0] * phi[1];
        return phi;
    }

    std::array<double, 3> MartinCoeff::linear_solve(const std::array<std::array<double, 3>, 3> &A,
                                                    const std::array<double, 3> &b) {
        std::array<double, 3> x{};
        if (std::abs(A[0][0]) > std::abs(A[0][1])) {
            x[0] = b[0] / A[0][0];
            auto denominator = -(A[1][2] * A[2][1] - A[1][1] * A[2][2]);
            x[1] = A[2][2] * b[1] - A[1][2] * b[2] + A[1][2] * A[2][0] * x[0] - A[1][0] * A[2][2] * x[0];
            x[1] /= denominator;
            x[2] = -A[2][1] * b[1] + A[1][1] * b[2] - A[1][1] * A[2][0] * x[0] + A[1][0] * A[2][1] * x[0];
            x[2] /= denominator;
        } else {
            x[1] = b[0] / A[0][1];
            auto denominator = -(A[1][2] * A[2][0] - A[1][0] * A[2][2]);
            x[0] = A[1][2] * A[2][1] * b[0] - A[1][1] * A[2][2] * b[0] + A[2][2] * b[1] - A[1][2] * b[2];
            x[0] /= denominator;
            x[2] = A[1][1] * A[2][0] * b[0] - A[1][0] * A[2][1] * b[0] - A[2][0] * b[1] + A[1][0] * b[2];
            x[2] /= denominator;
        }
        return x;
    }
}
