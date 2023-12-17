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

const static double pi = std::numbers::pi;
const static double pi2 = std::numbers::pi * std::numbers::pi;

namespace zlIIR {
    MartinCoeff::coeffs MartinCoeff::get1LowPass(double w0) {
        std::array<double, 3> a{}, b{};
        auto fc = w0 / pi;
        auto fm = 0.5 * std::sqrt(fc * fc + 1);
        auto phim = 1 - std::cos(pi * fm);

        a[0] = 1;
        a[1] = -std::exp(-w0);
        a[2] = 0;

        auto alpha = -2 * a[1] / std::pow(1 + a[1], 2);
        auto k = (fc * fc) / (fc * fc + fm * fm);
        auto beta = k * alpha + (k - 1) / phim;
        auto b_temp = -beta / (1 + beta + std::sqrt(1 + 2 * beta));

        b[0] = (1 + a[1]) / (1 + b_temp);
        b[1] = b_temp * b[0];
        b[2] = 0;
        return {a, b};
    }

    MartinCoeff::coeffs MartinCoeff::get1HighPass(double w0) {
        std::array<double, 3> a{}, b{};
        auto wm = w0 * 0.5;
        std::array<double, 2> phim = {1 - std::pow(std::sin(wm / 2), 2), std::pow(std::sin(wm / 2), 2)};

        a[0] = 1;
        a[1] = -std::exp(-w0);
        a[2] = 0;

        std::array<double, 2> A = {std::pow(a[0] + a[1], 2), std::pow(a[0] - a[1], 2)};
        auto B1 = (wm * wm) / (wm * wm + w0 * w0) * (A[0] * phim[0] + A[1] * phim[1]) / phim[1];
        auto b0 = 0.5 * std::sqrt(B1);

        b[0] = b0;
        b[1] = -b0;
        b[2] = 0;
        return {a, b};
    }

    MartinCoeff::coeffs MartinCoeff::get1LowShelf(double w0, double g) {
        auto [a, b] = get1HighShelf(w0, g);

        auto A = std::sqrt(g);
        std::array<double, 3> _a{b[0] / A, b[1] / A, b[2] / A};
        std::array<double, 3> _b{a[0] * A, a[1] * A, a[2] * A};
        return {_a, _b};
    }

    MartinCoeff::coeffs MartinCoeff::get1HighShelf(double w0, double g) {
        std::array<double, 3> a{}, b{};
        auto fc = w0 / pi;
        auto fm = fc * 0.75;
        auto phim = 1 - std::cos(pi * fm);
        auto alpha = 2 / pi2 * (1 / std::pow(fm, 2) + 1 / g / std::pow(fc, 2)) - 1 / phim;
        auto beta = 2 / pi2 * (1 / std::pow(fm, 2) + g / std::pow(fc, 2)) - 1 / phim;

        a[0] = 1;
        a[1] = -alpha / (1 + alpha + std::sqrt(1 + 2 * alpha));
        a[2] = 0;

        auto b_temp = -beta / (1 + beta + std::sqrt(1 + 2 * beta));

        b[0] = (1 + a[1]) / (1 + b_temp);
        b[1] = b_temp * b[0];
        b[2] = 0;
        return {a, b};
    }
}