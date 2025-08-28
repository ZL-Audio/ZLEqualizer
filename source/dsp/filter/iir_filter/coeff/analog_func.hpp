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

namespace zldsp::filter {
    class AnalogFunc {
    public:
        static double get2LowPassMagnitude2(double w0, double q, double w);

        static double get2HighPassMagnitude2(double w0, double q, double w);

        static double get2BandPassMagnitude2(double w0, double q, double w);

        static double get2NotchMagnitude2(double w0, double q, double w);

        static double get2PeakMagnitude2(double w0, double g, double q, double w);

        static double get2TiltShelfMagnitude2(double w0, double g, double q, double w);

    private:
        static double get2Magnitude2(const std::array<double, 6> &coeff, double w);
    };
}
