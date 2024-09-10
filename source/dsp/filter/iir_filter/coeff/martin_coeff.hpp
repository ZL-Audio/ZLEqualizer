// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLFILTER_MARTIN_COEFFS_H
#define ZLFILTER_MARTIN_COEFFS_H

#include "analog_func.hpp"
#include <numbers>

namespace zlFilter {
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
        constexpr static double piHalf = std::numbers::pi * 0.5;
        constexpr static double pi = std::numbers::pi;
        constexpr static double pi2 = std::numbers::pi * std::numbers::pi;

        static std::array<double, 3> solve_a(double w0, double b, double c = 1);

        static std::array<double, 3> get_AB(const std::array<double, 3> &a);

        static bool check_AB(const std::array<double, 3> &A);

        static std::array<double, 3> get_ab(const std::array<double, 3> &A);

        static std::array<double, 3> get_phi(double w);

        static std::array<double, 3> linear_solve(const std::array<std::array<double, 3>, 3> &A,
                                                  const std::array<double, 3> &b);
    };
}

#endif //ZLFILTER_MARTIN_COEFFS_H
