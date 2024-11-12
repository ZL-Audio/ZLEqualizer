// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLFILTER_HELPERS_HPP
#define ZLFILTER_HELPERS_HPP

#include <cmath>
#include <array>
#include <numeric>
#include <numbers>

namespace zlFilter {
    constexpr static double pi = std::numbers::pi;
    constexpr static double ppi = 2 * std::numbers::pi;

    enum FilterType {
        peak, lowShelf, lowPass, highShelf, highPass,
        notch, bandPass, tiltShelf, bandShelf,
    };

    enum FilterStructure {
        iir, svf, parallel
    };

    inline double dot_product(const std::array<double, 3> &x, const std::array<double, 3> &y) {
        return std::inner_product(x.begin(), x.end(), y.begin(), 0.0);
    }

    inline double gain_to_db(const double gain) {
        return std::log10(std::max(std::abs(gain), 1e-16)) * 20;
    }

    inline double db_to_gain(const double db) {
        return std::pow(10, db * 0.05);
    }

    inline std::array<double, 2> get_bandwidth(const double w0, const double q) {
        const auto bw = 2 * std::asinh(0.5 / q) / std::log(2);
        const auto scale = std::pow(2, bw / 2);
        return {w0 / scale, w0 * scale};
    }
}

#endif //ZLFILTER_HELPERS_HPP
