// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <array>
#include <numeric>
#include <numbers>

namespace zldsp::filter {
    constexpr static double pi = std::numbers::pi;
    constexpr static double ppi = 2 * std::numbers::pi;

    enum FilterType {
        kPeak, kLowShelf, kLowPass, kHighShelf, kHighPass,
        kNotch, kBandPass, kTiltShelf, kBandShelf,
    };

    enum FilterStructure {
        kIIR, kSVF, kParallel
    };

    inline double dotProduct(const std::array<double, 3> &x, const std::array<double, 3> &y) {
        return std::inner_product(x.begin(), x.end(), y.begin(), 0.0);
    }

    inline double gainToDB(const double gain) {
        return std::log10(std::max(std::abs(gain), 1e-16)) * 20;
    }

    inline double dbToGain(const double db) {
        return std::pow(10, db * 0.05);
    }

    inline std::array<double, 2> getBandwidth(const double w0, const double q) {
        const auto bw = 2 * std::asinh(0.5 / q) / std::log(2);
        const auto scale = std::pow(2, bw / 2);
        return {w0 / scale, w0 * scale};
    }
}
