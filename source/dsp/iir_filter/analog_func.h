// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_ANALOG_FUNC_H
#define ZLEQUALIZER_ANALOG_FUNC_H

#include <cmath>
#include <array>

namespace zlIIR {
    using coeff2  = std::array<double, 2>;
    using coeff3 = std::array<double, 3>;
    using coeff22 = std::tuple<coeff2, coeff2>;
    using coeff33 = std::tuple<coeff3, coeff3>;

    class AnalogFunc {
    public:
        static double get2LowPassMagnitude2(double w0, double q, double w);

        static double get2HighPassMagnitude2(double w0, double q, double w);

        static double get2BandPassMagnitude2(double w0, double q, double w);

        static double get2NotchMagnitude2(double w0, double q, double w);

        static double get2PeakMagnitude2(double w0, double g, double q, double w);

        static double get2LowShelfMagnitude2(double w0, double g, double q, double w);

        static double get2HighShelfMagnitude2(double w0, double g, double q, double w);

    private:
        static double get2Magnitude2(std::array<double, 6> coeff, double w);
    };
}

#endif //ZLEQUALIZER_ANALOG_FUNC_H
