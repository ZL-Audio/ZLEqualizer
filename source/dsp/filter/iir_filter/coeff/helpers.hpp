// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

//
// Created by Zishu Liu on 12/17/23.
//

#ifndef ZLEQUALIZER_HELPERS_HPP
#define ZLEQUALIZER_HELPERS_HPP

#include <cmath>
#include <array>
#include <numbers>
#include <numeric>
#include <vector>
#include <tuple>

namespace zlFilter {
    enum FilterType {
        peak, lowShelf, lowPass, highShelf, highPass,
        notch, bandPass, tiltShelf, bandShelf,
    };

    using coeff2 = std::array<double, 2>;
    using coeff3 = std::array<double, 3>;
    using coeff22 = std::tuple<coeff2, coeff2>;
    using coeff33 = std::tuple<coeff3, coeff3>;

    inline double dot_product(coeff3 x, coeff3 y) {
        return std::inner_product(x.begin(), x.end(), y.begin(), 0.0);
    }

    inline double gain_to_db(double gain) {
        return std::log10(gain) * 20;
    }

    inline double db_to_gain(double db) {
        return std::pow(10, db * 0.05);
    }

    inline std::tuple<double, double> get_bandwidth(double w0, double q) {
        auto bw = 2 * std::asinh(0.5 / q) / std::log(2);
        auto scale = std::pow(2, bw / 2);
        return {w0 / scale, w0 * scale};
    }
}

#endif //ZLEQUALIZER_HELPERS_HPP
