// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ideal_coeff.hpp"
#include <cmath>

#include "../../helpers.hpp"

namespace zldsp::filter {
    std::array<double, 3> IdealCoeff::get1LowPass(double w0) {
        w0 = w0 * ppi;
        return {w0, 0.0, w0};
    }

    std::array<double, 3> IdealCoeff::get1HighPass(double w0) {
        w0 = w0 * ppi;
        return {w0, 1.0, 0.0};;
    }

    std::array<double, 3> IdealCoeff::get1TiltShelf(double w0, const double g) {
        w0 = w0 * ppi;
        const auto A = std::exp2(g * kDbToExp2Sqrt);;
        return {A * w0, A, w0};
    }

    std::array<double, 3> IdealCoeff::get1LowShelf(double w0, const double g) {
        w0 = w0 * ppi;
        const auto A = std::exp2(g * kDbToExp2Sqrt);;
        return {w0 / A, 1.0, w0 * A};
    }

    std::array<double, 3> IdealCoeff::get1HighShelf(double w0, const double g) {
        w0 = w0 * ppi;
        const auto A = std::exp2(g * kDbToExp2Sqrt);;
        return {w0 * A, A * A, w0 * A};
    }

    std::array<double, 5> IdealCoeff::get2LowPass(double w0, const double q) {
        w0 = w0 * ppi;
        const auto w02 = w0 * w0;
        return {w0 / q, w02, 0, 0, w02};
    }

    std::array<double, 5> IdealCoeff::get2HighPass(double w0, const double q) {
        w0 = w0 * ppi;
        return {w0 / q, w0 * w0, 1, 0, 0};
    }

    std::array<double, 5> IdealCoeff::get2BandPass(double w0, const double q) {
        w0 = w0 * ppi;
        return {w0 / q, w0 * w0, 0, w0 / q, 0};
    }

    std::array<double, 5> IdealCoeff::get2Notch(double w0, const double q) {
        w0 = w0 * ppi;
        const auto w02 = w0 * w0;
        return {w0 / q, w02, 1, 0, w02};
    }

    std::array<double, 5> IdealCoeff::get2Peak(double w0, const double g, const double q) {
        w0 = w0 * ppi;
        const auto w02 = w0 * w0;
        const auto A = std::exp2(g * kDbToExp2Sqrt);;
        return {w0 / A / q, w02, 1, w0 * A / q, w02};
    }

    std::array<double, 5> IdealCoeff::get2TiltShelf(double w0, const double g, const double q) {
        w0 = w0 * ppi;
        const auto A = std::exp2(g * kDbToExp2Sqrt);;
        const auto Awq = std::sqrt(A) * w0 / q;
        const auto w02 = w0 * w0;
        return {Awq, A * w02, A, Awq, w02};
    }

    std::array<double, 5> IdealCoeff::get2LowShelf(double w0, const double g, const double q) {
        w0 = w0 * ppi;
        const auto A = std::exp2(g * kDbToExp2Sqrt);;
        const auto Awq = std::sqrt(A) * w0 / q;
        const auto w02 = w0 * w0;
        return {Awq / A, w02 / A, 1.0, Awq, A * w02};
    }

    std::array<double, 5> IdealCoeff::get2HighShelf(double w0, const double g, const double q) {
        w0 = w0 * ppi;
        const auto A = std::exp2(g * kDbToExp2Sqrt);
        const auto Awq = std::sqrt(A) * w0 / q;
        const auto w02 = w0 * w0;
        return {Awq, A * w02, A * A, A * Awq, A * w02};
    }
}
