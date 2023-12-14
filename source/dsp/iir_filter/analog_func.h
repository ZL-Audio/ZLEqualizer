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

namespace zl_iir {
    class AnalogFunc {
    public:
        static double getLowPassMagnitude2(double w0, double q, double w);

        static double getHighPassMagnitude2(double w0, double q, double w);

        static double getBandPassMagnitude2(double w0, double q, double w);

        static double getNotchMagnitude2(double w0, double q, double w);

        static double getPeakMagnitude2(double w0, double g, double q, double w);

        static double getLowShelfMagnitude2(double w0, double g, double q, double w);

        static double getHighShelfMagnitude2(double w0, double g, double q, double w);

    private:
        static double getMagnitude2(std::array<double, 6> coeff, double w);
    };
}

#endif //ZLEQUALIZER_ANALOG_FUNC_H
