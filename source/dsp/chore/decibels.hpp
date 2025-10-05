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
#include <algorithm>

namespace zldsp::chore {
    template <typename FloatType>
    FloatType decibelsToGain(const FloatType value) {
        return std::pow(FloatType(10), value * FloatType(0.05));
    }

    template <typename FloatType>
    FloatType gainToDecibels(const FloatType value) {
        return FloatType(20) * std::log10(std::max(value, FloatType(1e-12)));
    }

    template <typename FloatType>
    FloatType squareGainToDecibels(const FloatType value) {
        return FloatType(10) * std::log10(std::max(value, FloatType(1e-24)));
    }
}
