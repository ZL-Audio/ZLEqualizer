// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ideal_coeff.hpp"
#include <cmath>

namespace zldsp::filter {
    std::array<double, 4> IdealCoeff::get1LowPass(const double w0) {
        return {1.0, w0, 0.0, w0};
    }

    std::array<double, 4> IdealCoeff::get1HighPass(const double w0) {
        return {1.0, w0, 1.0, 0.0};;
    }

    std::array<double, 4> IdealCoeff::get1TiltShelf(const double w0, const double g) {
        const auto A = std::sqrt(g);
        return {1.0, A * w0, A, w0};
    }

    std::array<double, 4> IdealCoeff::get1LowShelf(const double w0, const double g) {
        const auto A = std::sqrt(g);
        return {1.0, w0 / A, 1.0, w0 * A};
    }

    std::array<double, 4> IdealCoeff::get1HighShelf(const double w0, const double g) {
        const auto A = std::sqrt(g);
        return {1.0 / A, w0, A, w0};
    }

    std::array<double, 6> IdealCoeff::get2LowPass(const double w0, const double q) {
        const auto w02 = w0 * w0;
        return {1.0, w0 / q, w02, 0, 0, w02};
    }

    std::array<double, 6> IdealCoeff::get2HighPass(const double w0, const double q) {
        return {1, w0 / q, w0 * w0, 1, 0, 0};
    }

    std::array<double, 6> IdealCoeff::get2BandPass(const double w0, const double q) {
        return {1, w0 / q, w0 * w0, 0, w0 / q, 0};
    }

    std::array<double, 6> IdealCoeff::get2Notch(const double w0, const double q) {
        const auto w02 = w0 * w0;
        return {1, w0 / q, w02, 1, 0, w02};
    }

    std::array<double, 6> IdealCoeff::get2Peak(const double w0, const double g, const double q) {
        const auto w02 = w0 * w0;
        const auto A = std::sqrt(g);
        return {1.0, w0 / A / q, w02, 1, w0 * A / q, w02};
    }

    std::array<double, 6> IdealCoeff::get2TiltShelf(const double w0, const double g, const double q) {
        const auto A = std::sqrt(g);
        const auto Awq = std::sqrt(A) * w0 / q;
        const auto w02 = w0 * w0;
        return {1, Awq, A * w02, A, Awq, w02};
    }

    std::array<double, 6> IdealCoeff::get2LowShelf(const double w0, const double g, const double q) {
        const auto A = std::sqrt(g);
        const auto Awq = std::sqrt(A) * w0 / q;
        const auto w02 = w0 * w0;
        return {A, Awq,  w02, A, A * Awq, A * A * w02};
    }

    std::array<double, 6> IdealCoeff::get2HighShelf(const double w0, const double g, const double q) {
        const auto A = std::sqrt(g);
        const auto Awq = std::sqrt(A) * w0 / q;
        const auto w02 = w0 * w0;
        return {1, Awq, A * w02, A * A, A * Awq, A * w02};
    }
} // zldsp::filter
