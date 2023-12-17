// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_MARTIN_COEFFS_H
#define ZLEQUALIZER_MARTIN_COEFFS_H

#include <cmath>
#include <array>
#include <numbers>

namespace zlIIR {
    class MartinCoeff {
    public:
        using coeffs = std::tuple<std::array<double, 3>, std::array<double, 3>>;

        static coeffs get1LowPass(double w0);

        static coeffs get1HighPass(double w0);

        static coeffs get1LowShelf(double w0, double g);

        static coeffs get1HighShelf(double w0, double g);

    };
}

#endif //ZLEQUALIZER_MARTIN_COEFFS_H
