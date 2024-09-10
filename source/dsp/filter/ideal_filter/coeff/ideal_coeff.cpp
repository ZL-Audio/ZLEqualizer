// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ideal_coeff.hpp"

namespace zlFilter {
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
        const auto gq = std::sqrt(g) / q;
        return {1, w0 / gq, w02, 1, w0 * gq, w02};
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
        return {A, Awq, A * w02, A, A * Awq, A * w02};
    }

    std::array<double, 6> IdealCoeff::get2HighShelf(const double w0, const double g, const double q) {
        const auto A = std::sqrt(g);
        const auto Awq = std::sqrt(A) * w0 / q;
        const auto w02 = w0 * w0;
        return {1, Awq, A * w02, A * A, A * Awq, A * w02};
    }
} // zlFilter
