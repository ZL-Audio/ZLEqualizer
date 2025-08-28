// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "analog_func.hpp"

namespace zldsp::filter {
    double AnalogFunc::get2Magnitude2(const std::array<double, 6> &coeff, const double w) {
        const auto w_2 = w * w;
        const auto t1 = coeff[2] - coeff[0] * w_2;
        const auto denominator = coeff[1] * coeff[1] * w_2 + t1 * t1;
        const auto t2 = coeff[5] - coeff[3] * w_2;
        const auto numerator = coeff[4] * coeff[4] * w_2 + t2 * t2;
        return numerator / denominator;
    }

    double AnalogFunc::get2LowPassMagnitude2(const double w0, const double q, const double w) {
        return get2Magnitude2({1, w0 / q, w0 * w0, 0, 0, w0 * w0}, w);
    }

    double AnalogFunc::get2HighPassMagnitude2(const double w0, const double q, const double w) {
        return get2Magnitude2({1, w0 / q, w0 * w0, 1, 0, 0}, w);
    }

    double AnalogFunc::get2BandPassMagnitude2(const double w0, const double q, const double w) {
        return get2Magnitude2({1, w0 / q, w0 * w0, 0, w0 / q, 0}, w);
    }

    double AnalogFunc::get2NotchMagnitude2(const double w0, const double q, const double w) {
        return get2Magnitude2({1, w0 / q, w0 * w0, 1, 0, w0 * w0}, w);
    }

    double AnalogFunc::get2PeakMagnitude2(const double w0, double g, const double q, const double w) {
        return get2Magnitude2({1, w0 / std::sqrt(g) / q, w0 * w0, 1, w0 * std::sqrt(g) / q, w0 * w0}, w);
    }

    double AnalogFunc::get2TiltShelfMagnitude2(const double w0, double g, const double q, const double w) {
        const auto A = std::sqrt(g);
        return get2Magnitude2({1, std::sqrt(A) * w0 / q, A * w0 * w0, A, std::sqrt(A) * w0 / q, w0 * w0}, w);
    }
}
