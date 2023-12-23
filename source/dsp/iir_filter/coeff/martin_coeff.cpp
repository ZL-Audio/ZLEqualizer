// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

// Reference:
// Matched One-Pole Digital Shelving Filters, Martin Vicanek
// Matched Second Order Digital Filters, Martin Vicanek

#include "martin_coeff.h"

const static double piHalf = std::numbers::pi * 0.5;
const static double pi = std::numbers::pi;
const static double pi2 = std::numbers::pi * std::numbers::pi;

namespace zlIIR {
    coeff22 MartinCoeff::get1LowPass(double w0) {
        coeff2 a{}, b{};
        auto fc = w0 / pi;
        auto fm = 0.5 * std::sqrt(fc * fc + 1);
        auto phim = 1 - std::cos(pi * fm);

        a[0] = 1;
        a[1] = -std::exp(-w0);

        auto alpha = -2 * a[1] / std::pow(1 + a[1], 2);
        auto k = (fc * fc) / (fc * fc + fm * fm);
        auto beta = k * alpha + (k - 1) / phim;
        auto b_temp = -beta / (1 + beta + std::sqrt(1 + 2 * beta));

        b[0] = (1 + a[1]) / (1 + b_temp);
        b[1] = b_temp * b[0];
        return {a, b};
    }

    coeff22 MartinCoeff::get1HighPass(double w0) {
        coeff2 a{}, b{};
        auto wm = w0 * 0.5;
        coeff2 phim = {1 - std::pow(std::sin(wm / 2), 2), std::pow(std::sin(wm / 2), 2)};

        a[0] = 1;
        a[1] = -std::exp(-w0);

        coeff2 A = {std::pow(a[0] + a[1], 2), std::pow(a[0] - a[1], 2)};
        auto B1 = (wm * wm) / (wm * wm + w0 * w0) * (A[0] * phim[0] + A[1] * phim[1]) / phim[1];
        auto b0 = 0.5 * std::sqrt(B1);

        b[0] = b0;
        b[1] = -b0;
        return {a, b};
    }

    coeff22 MartinCoeff::get1TiltShelf(double w0, double g) {
        coeff2 a{}, b{};
        auto fc = w0 / pi;
        auto fm = fc * 0.75;
        auto phim = 1 - std::cos(pi * fm);
        auto alpha = 2 / pi2 * (1 / std::pow(fm, 2) + 1 / g / std::pow(fc, 2)) - 1 / phim;
        auto beta = 2 / pi2 * (1 / std::pow(fm, 2) + g / std::pow(fc, 2)) - 1 / phim;

        a[0] = 1;
        a[1] = -alpha / (1 + alpha + std::sqrt(1 + 2 * alpha));

        auto b_temp = -beta / (1 + beta + std::sqrt(1 + 2 * beta));

        b[0] = (1 + a[1]) / (1 + b_temp) / std::sqrt(g);
        b[1] = b_temp * b[0];
        return {a, b};
    }

    coeff22 MartinCoeff::get1LowShelf(double w0, double g) {
        auto [a, b] = get1TiltShelf(w0, 1.0 / g);

        auto A = std::sqrt(g);
        return {a, {b[0] * A, b[1] * A}};
    }

    coeff22 MartinCoeff::get1HighShelf(double w0, double g) {
        auto [a, b] = get1TiltShelf(w0, g);

        auto A = std::sqrt(g);
        return {a, {b[0] * A, b[1] * A}};
    }

    coeff33 MartinCoeff::get2LowPass(double w0, double q) {
        auto a = solve_a(w0, 0.5 / q, 1);
        auto A = get_AB(a);
        coeff3 ws{};
        if (w0 > pi / 32) {
            ws = {0, 0.5 * w0, w0};
        } else {
            ws = {pi, w0, 0.5 * (pi + w0)};
        }
        std::array<coeff3, 3> phi{};
        coeff3 res{};
        for (size_t i = 0; i < 3; ++i) {
            phi[i] = get_phi(ws[i]);
            res[i] = AnalogFunc::get2LowPassMagnitude2(w0, q, ws[i]) * dot_product(phi[i], A);
        }
        auto B = linear_solve(phi, res);
        auto b = get_ab(B);
        return {a, b};
    }

    coeff33 MartinCoeff::get2HighPass(double w0, double q) {
        auto a = solve_a(w0, 0.5 / q, 1);
        auto A = get_AB(a);
        auto phi0 = get_phi(w0);

        coeff3 b{};
        b[0] = q * std::sqrt(dot_product(A, phi0)) / 4 / phi0[1];
        b[1] = -2 * b[0];
        b[2] = b[0];
        return {a, b};
    }

    coeff33 MartinCoeff::get2BandPass(double w0, double q) {
        auto a = solve_a(w0, 0.5 / q);
        auto A = get_AB(a);

        if (w0 > pi / 32) {
            auto phi0 = get_phi(w0);
            auto R1 = dot_product(phi0, A);
            auto R2 = dot_product({-1, 1, 4 * (phi0[0] - phi0[1])}, A);

            coeff3 B{};
            B[0] = 0.0;
            B[2] = (R1 - R2 * phi0[1]) / 4 / std::pow(phi0[1], 2);
            B[1] = R2 + 4 * (phi0[1] - phi0[0]) * B[2];

            auto b = get_ab(B);
            return {a, b};
        } else {
            auto [w1, w2] = get_bandwidth(w0, q);
            coeff3 ws{0, w0, w0 > piHalf ? w1 : w2};
            coeff3 B{-1, -1, -1};
            size_t trial = 0;
            while (!check_AB(B) && trial < 10) {
                trial += 1;
                std::array<coeff3, 3> phi{};
                coeff3 res{};
                for (size_t i = 0; i < 3; ++i) {
                    phi[i] = get_phi(ws[i]);
                    res[i] = AnalogFunc::get2BandPassMagnitude2(w0, q, ws[i]) * dot_product(phi[i], A);
                }
                B = linear_solve(phi, res);
                ws[2] = w0 > piHalf ? 0.9 * ws[2] : 0.9 * ws[2] + 0.1 * pi;
            }
            auto b = get_ab(B);
            return {a, b};
        }
    }

    coeff33 MartinCoeff::get2Notch(double w0, double q) {
        coeff3 b{};
        if (w0 < pi) {
            b = {1, -2 * std::cos(w0), 1};
        } else {
            b = {1, -2 * std::sinh(w0), 1};
        }
        auto B = get_AB(b);

        auto [w1, w2] = get_bandwidth(w0, q);
        coeff3 ws{0, w1, w2 < pi ? w2 : 0.5 * (w0 + w1)};

        std::array<coeff3, 3> phi{};
        coeff3 res{};
        for (size_t i = 0; i < 3; ++i) {
            phi[i] = get_phi(ws[i]);
            res[i] = dot_product(phi[i], B) / AnalogFunc::get2NotchMagnitude2(w0, q, ws[i]);
        }

        auto A = linear_solve(phi, res);
        auto a = get_ab(A);
        return {a, b};
    }

    coeff33 MartinCoeff::get2Peak(double w0, double g, double q) {
        auto a = solve_a(w0, 0.5 / std::sqrt(g) / q);
        auto A = get_AB(a);
        auto phi0 = get_phi(w0);

        auto R1 = dot_product(A, phi0) * std::pow(g, 2);
        auto R2 = (-A[0] + A[1] + 4 * (phi0[0] - phi0[1]) * A[2]) * std::pow(g, 2);

        coeff3 B{A[0], 0, 0};
        B[2] = (R1 - R2 * phi0[1] - B[0]) / (4 * std::pow(phi0[1], 2));
        B[1] = R2 + B[0] + 4 * (phi0[1] - phi0[0]) * B[2];
        auto b = get_ab(B);

        return {a, b};
    }

    coeff33 MartinCoeff::get2TiltShelf(double w0, double g, double q) {
        bool reverse_ab = (g > 1);
        if (g > 1) {
            g = 1 / g;
        }
        auto g_sqrt = std::sqrt(g);
        auto a = solve_a(w0, std::sqrt(g_sqrt) / 2 / q, std::sqrt(g_sqrt));
        auto A = get_AB(a);

        auto c2 = g_sqrt * (-1 + 2 * q * q);
        auto c0 = c2 * std::pow(w0, 4);
        auto c1 = -2 * (1 + g) * std::pow(q * w0, 2);
        auto delta = c1 * c1 - 4 * c0 * c2;
        coeff3 ws{};
        if (delta <= 0) {
            ws = {0, w0 / 2, w0};
        } else {
            delta = std::sqrt(delta);
            auto sol1 = (-c1 + delta) / 2 / c2;
            auto sol2 = (-c1 - delta) / 2 / c2;
            if (sol1 < 0 || sol2 < 0) {
                ws = {0, w0 / 2, w0};
            } else {
                auto w1 = std::sqrt((-c1 + delta) / 2 / c2);
                auto w2 = std::sqrt((-c1 - delta) / 2 / c2);
                if (w1 < pi || w2 < pi) {
                    ws = {0, std::min(w1, w2), std::min(std::max(w1, w2), pi)};
                } else {
                    ws = {0, piHalf, pi};
                }
            }
        }
        coeff3 B{-1, -1, -1};
        size_t trial = 0;
        while (!check_AB(B) && trial < 20) {
            trial += 1;
            std::array<coeff3, 3> phi{};
            coeff3 res{};
            for (size_t i = 0; i < 3; ++i) {
                phi[i] = get_phi(ws[i]);
                res[i] = AnalogFunc::get2TiltShelfMagnitude2(w0, g, q, ws[i]) * dot_product(phi[i], A);
            }
            B = linear_solve(phi, res);
            ws[2] = 0.5 * (ws[2] + pi);
        }
        auto b = get_ab(B);
        if (reverse_ab) {
            return {b, a};
        } else {
            return {a, b};
        }
    }

    coeff33 MartinCoeff::get2LowShelf(double w0, double g, double q) {
        auto [a, b] = get2TiltShelf(w0, 1 / g, q);

        auto A = std::sqrt(g);
        return {a, {b[0] * A, b[1] * A, b[2] * A}};
    }

    coeff33 MartinCoeff::get2HighShelf(double w0, double g, double q) {
        auto [a, b] = get2TiltShelf(w0, g, q);

        auto A = std::sqrt(g);
        return {a, {b[0] * A, b[1] * A, b[2] * A}};
    }

    coeff3 MartinCoeff::solve_a(double w0, double b, double c) {
        coeff3 a{};
        a[0] = 1.0;
        if (b <= c) {
            a[1] = -2 * std::exp(-b * w0) * std::cos(std::sqrt(c * c - b * b) * w0);
        } else {
            a[1] = -2 * std::exp(-b * w0) * std::cosh(std::sqrt(b * b - c * c) * w0);
        }
        a[2] = std::exp(-2 * b * w0);
        return a;
    }

    coeff3 MartinCoeff::get_AB(coeff3 a) {
        coeff3 A{};
        A[0] = std::pow(a[0] + a[1] + a[2], 2);
        A[1] = std::pow(a[0] - a[1] + a[2], 2);
        A[2] = -4 * a[2];
        return A;
    }

    bool MartinCoeff::check_AB(zlIIR::coeff3 A) {
        return A[0] > 0 && A[1] > 0 && std::pow(0.5 * (std::sqrt(A[0]) + std::sqrt(A[1])), 2) + A[2] > 0;
    }

    coeff3 MartinCoeff::get_ab(coeff3 A) {
        coeff3 a{};
        auto W = 0.5 * (std::sqrt(std::max(A[0], 0.0)) + std::sqrt(std::max(A[1], 0.0)));
        auto temp = std::max(W * W + A[2], 0.0);
        a[0] = 0.5 * (W + std::sqrt(temp));
        a[1] = 0.5 * (std::sqrt(A[0]) - std::sqrt(A[1]));
        a[2] = -A[2] / 4 / a[0];
        return a;
    }

    coeff3 MartinCoeff::get_phi(double w) {
        coeff3 phi{};
        phi[0] = 1 - std::pow(std::sin(w / 2), 2);
        phi[1] = 1 - phi[0];
        phi[2] = 4 * phi[0] * phi[1];
        return phi;
    }

    coeff3 MartinCoeff::linear_solve(std::array<coeff3, 3> A, coeff3 b) {
        coeff3 x{};
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