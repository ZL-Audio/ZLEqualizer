// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLFILTER_CORRECTION_HELPER_HPP
#define ZLFILTER_CORRECTION_HELPER_HPP

#include <cmath>
#include <array>
#include <numeric>
#include <numbers>

namespace zlFilter {
    template<typename FloatType>
    void calculateWsForPrototype(std::vector<std::complex<FloatType>> &ws) {
        const auto delta = static_cast<FloatType>(pi) / static_cast<double>(ws.size() - 1);
        double w = 0.f;
        for (size_t i = 0; i < ws.size(); ++i) {
            ws[i] = std::complex<FloatType>(0.f, static_cast<FloatType>(w));
            w += delta;
        }
    }

    template<typename FloatType>
    void calculateWsForBiquad(std::vector<std::complex<FloatType>> &ws) {
        const auto delta = static_cast<FloatType>(pi) / static_cast<double>(ws.size() - 1);
        double w = 0.f;
        for (size_t i = 0; i < ws.size(); ++i) {
            ws[i] = std::exp(std::complex<FloatType>(0.f, -static_cast<FloatType>(w)));
            w += delta;
        }
    }
}

#endif //ZLFILTER_CORRECTION_HELPER_HPP
