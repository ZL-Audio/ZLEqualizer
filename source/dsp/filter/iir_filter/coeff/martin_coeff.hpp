// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "analog_func.hpp"
#include <numbers>

namespace zldsp::filter {
    class MartinCoeff {
    public:
        static std::array<double, 4> get1LowPass(double w0);

        static std::array<double, 4> get1HighPass(double w0);

        static std::array<double, 4> get1TiltShelf(double w0, double g);

        static std::array<double, 4> get1LowShelf(double w0, double g);

        static std::array<double, 4> get1HighShelf(double w0, double g);

        static std::array<double, 6> get2LowPass(double w0, double q);

        static std::array<double, 6> get2HighPass(double w0, double q);

        static std::array<double, 6> get2BandPass(double w0, double q);

        static std::array<double, 6> get2Notch(double w0, double q);

        static std::array<double, 6> get2Peak(double w0, double g, double q);

        static std::array<double, 6> get2TiltShelf(double w0, double g, double q);

        static std::array<double, 6> get2LowShelf(double w0, double g, double q);

        static std::array<double, 6> get2HighShelf(double w0, double g, double q);

    private:
        constexpr static double kPiHalf = std::numbers::pi * 0.5;
        constexpr static double kPi = std::numbers::pi;
        constexpr static double kPi2 = std::numbers::pi * std::numbers::pi;

        static std::array<double, 3> solve_a(double w0, double b, double c = 1);

        static std::array<double, 3> get_AB(const std::array<double, 3> &a);

        static bool check_AB(const std::array<double, 3> &A);

        static std::array<double, 3> get_ab(const std::array<double, 3> &A);

        static std::array<double, 3> get_phi(double w);

        static std::array<double, 3> linear_solve(const std::array<std::array<double, 3>, 3> &A,
                                                  const std::array<double, 3> &b);
    };
}
