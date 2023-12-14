// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "analog_func.h"

namespace zl_iir {
    double AnalogFunc::getMagnitude2(std::array<double, 6> coeff, double w) {
        auto w_2 = w * w;
        auto denominator = std::pow(coeff[1], 2) * w_2 + std::pow(coeff[2] - coeff[0] * w_2, 2);
        auto numerator = std::pow(coeff[4], 2) * w_2 + std::pow(coeff[5] - coeff[3] * w_2, 2);
        return numerator / denominator;
    }

    double AnalogFunc::getLowPassMagnitude2(double w0, double q, double w) {
        return getMagnitude2({1, w0 / q, w0 * w0, 0, 0, w0 * w0}, w);
    }

    double AnalogFunc::getHighPassMagnitude2(double w0, double q, double w) {
        return getMagnitude2({1, w0 / q, w0 * w0, 1, 0, 0}, w);
    }

    double AnalogFunc::getBandPassMagnitude2(double w0, double q, double w) {
        return getMagnitude2({1, w0 / q, w0 * w0, 0, w0 / q, 0}, w);
    }

    double AnalogFunc::getNotchMagnitude2(double w0, double q, double w) {
        return getMagnitude2({1, w0 / q, w0 * w0, 1, 0, w0 * w0}, w);
    }

    double AnalogFunc::getPeakMagnitude2(double w0, double g, double q, double w) {
        return getMagnitude2({1, w0 / std::sqrt(g) / q, w0 * w0, 1, w0 * std::sqrt(g) / q, w0 * w0}, w);
    }

    double AnalogFunc::getLowShelfMagnitude2(double w0, double g, double q, double w) {
        auto A = std::sqrt(g);
        return getMagnitude2({A, std::sqrt(A) * w0 / q, w0 * w0, A, A * std::sqrt(A) * w0 / q, A * w0 * w0}, w);
    }

    double AnalogFunc::getHighShelfMagnitude2(double w0, double g, double q, double w) {
        auto A = std::sqrt(g);
        return getMagnitude2({1, std::sqrt(A) * w0 / q, w0 * w0, A * A, A * std::sqrt(A) * w0 / q, A * w0 * w0}, w);
    }
}