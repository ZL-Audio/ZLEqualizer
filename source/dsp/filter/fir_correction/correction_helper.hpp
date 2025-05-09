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
    template<typename FloatType>
    void calculateWsForPrototype(std::vector<std::complex<FloatType> > &ws) {
        const auto delta = static_cast<FloatType>(pi) / static_cast<double>(ws.size() - 1);
        double w = 0.f;
        for (size_t i = 0; i < ws.size(); ++i) {
            ws[i] = std::complex<FloatType>(0.f, static_cast<FloatType>(w));
            w += delta;
        }
    }

    template<typename FloatType>
    void calculateWsForBiquad(std::vector<std::complex<FloatType> > &ws) {
        const auto delta = static_cast<FloatType>(pi) / static_cast<double>(ws.size() - 1);
        double w = 0.f;
        for (size_t i = 0; i < ws.size(); ++i) {
            ws[i] = std::exp(std::complex<FloatType>(0.f, -static_cast<FloatType>(w)));
            w += delta;
        }
    }
}
