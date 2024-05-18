// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "analog_func.hpp"

namespace zlIIR {
    double AnalogFunc::get2Magnitude2(std::array<double, 6> coeff, double w) {
        auto w_2 = w * w;
        auto denominator = std::pow(coeff[1], 2) * w_2 + std::pow(coeff[2] - coeff[0] * w_2, 2);
        auto numerator = std::pow(coeff[4], 2) * w_2 + std::pow(coeff[5] - coeff[3] * w_2, 2);
        return numerator / denominator;
    }

    double AnalogFunc::get2LowPassMagnitude2(double w0, double q, double w) {
        return get2Magnitude2({1, w0 / q, w0 * w0, 0, 0, w0 * w0}, w);
    }

    double AnalogFunc::get2HighPassMagnitude2(double w0, double q, double w) {
        return get2Magnitude2({1, w0 / q, w0 * w0, 1, 0, 0}, w);
    }

    double AnalogFunc::get2BandPassMagnitude2(double w0, double q, double w) {
        return get2Magnitude2({1, w0 / q, w0 * w0, 0, w0 / q, 0}, w);
    }

    double AnalogFunc::get2NotchMagnitude2(double w0, double q, double w) {
        return get2Magnitude2({1, w0 / q, w0 * w0, 1, 0, w0 * w0}, w);
    }

    double AnalogFunc::get2PeakMagnitude2(double w0, double g, double q, double w) {
        return get2Magnitude2({1, w0 / std::sqrt(g) / q, w0 * w0, 1, w0 * std::sqrt(g) / q, w0 * w0}, w);
    }

    double AnalogFunc::get2TiltShelfMagnitude2(double w0, double g, double q, double w) {
        auto A = std::sqrt(g);
        return get2Magnitude2({1, std::sqrt(A) * w0 / q, A * w0 * w0, A, std::sqrt(A) * w0 / q, w0 * w0}, w);
    }

//    double AnalogFunc::get2LowShelfMagnitude2(double w0, double g, double q, double w) {
//        auto A = std::sqrt(g);
//        return get2Magnitude2({A, A * std::sqrt(A) * w0 / q, A * A * w0 * w0, A, std::sqrt(A) * w0 / q, A * w0 * w0}, w);
//    }
//
//    double AnalogFunc::get2HighShelfMagnitude2(double w0, double g, double q, double w) {
//        auto A = std::sqrt(g);
//        return get2Magnitude2({1, std::sqrt(A) * w0 / q, w0 * w0, A * A, A * std::sqrt(A) * w0 / q, A * w0 * w0}, w);
//    }
}