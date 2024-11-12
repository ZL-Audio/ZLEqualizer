// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef FILTER_DESIGN_HPP
#define FILTER_DESIGN_HPP

#include "../helpers.hpp"

namespace zlFilter::FilterDesign {
    template<size_t ArraySize,
        std::array<double, 4>(*firstOrderFunc)(double w),
        std::array<double, 6>(*func)(double w, double q)>
    size_t updatePassCoeffs(const size_t n, const size_t startIdx,
                            const double w0, const double q0,
                            std::array<std::array<double, 6>, ArraySize> &coeffs) {
        if (n == 1) {
            const auto coeff = firstOrderFunc(w0);
            coeffs[startIdx] = {coeff[0], coeff[1], 0.0, coeff[2], coeff[3], 0.0};
            return 1;
        }
        const size_t number = n / 2;
        const auto theta0 = pi / static_cast<double>(number) / 4;
        const auto scale = std::pow(std::sqrt(2.0) * q0, 1 / static_cast<double>(number));
        const auto rescale_base = std::log10(std::sqrt(2.0) * q0) / std::pow(static_cast<double>(n), 1.5) * 12;
        for (size_t i = 0; i < number; i++) {
            const auto centered = static_cast<double>(i) - static_cast<double>(number) / 2 + 0.5;
            const auto rescale = centered * rescale_base;
            const auto theta = theta0 * static_cast<double>(2 * i + 1);
            const auto qs = 1.0 / 2.0 / std::cos(theta) * scale * std::pow(2, rescale);
            coeffs[i + startIdx] = func(w0, qs);
        }
        return number;
    }

    template<size_t ArraySize,
        std::array<double, 4>(*firstOrderFunc)(double w, double g),
        std::array<double, 6>(*func)(double w, double g, double q)>
    size_t updateShelfCoeffs(const size_t n, const size_t startIdx,
                             const double w0, const double g0, const double q0,
                             std::array<std::array<double, 6>, ArraySize> &coeffs) {
        if (n == 1) {
            const auto coeff = firstOrderFunc(w0, g0);
            coeffs[startIdx] = {coeff[0], coeff[1], 0.0, coeff[2], coeff[3], 0.0};
            return 1;
        }
        const size_t number = n / 2;
        const auto _g = std::pow(g0, 1.0 / static_cast<double>(number));
        const auto theta0 = pi / static_cast<double>(number) / 4;
        const auto scale = std::pow(std::sqrt(2.0) * q0, 1 / static_cast<double>(number));
        const auto rescale_base = std::log10(std::sqrt(2.0) * q0) / std::pow(static_cast<double>(n), 1.5) * 12;
        for (size_t i = 0; i < number; i++) {
            const auto centered = static_cast<double>(i) - static_cast<double>(number) / 2 + 0.5;
            const auto rescale = centered * rescale_base;
            const auto theta = theta0 * static_cast<double>(2 * i + 1);
            const auto _q = 1.0 / 2.0 / std::cos(theta) * scale * std::pow(2, rescale);
            coeffs[i + startIdx] = func(w0, _g, _q);
        }
        return number;
    }

    template<size_t ArraySize, std::array<double, 6>(*func)(double w, double q)>
    size_t updateBandPassCoeffs(const size_t n, const size_t startIdx,
                                const double w0, const double q0,
                                std::array<std::array<double, 6>, ArraySize> &coeffs) {
        if (n < 2) { return 0; }
        const size_t number = n / 2;
        const auto halfbw = std::asinh(0.5 / q0) / std::log(2);
        const auto w = w0 / std::pow(2, halfbw);
        const auto g = db_to_gain(-6 / static_cast<double>(n));
        const auto _q = std::sqrt(1 - g * g) * w * w0 / g / (w0 * w0 - w * w);

        const auto singleCoeff = func(w0, _q);
        for (size_t i = 0; i < n / 2; ++i) {
            coeffs[i + startIdx] = singleCoeff;
        }
        return number;
    }

    template<size_t ArraySize, std::array<double, 6>(*func)(double w, double q)>
    size_t updateNotchCoeffs(const size_t n, const size_t startIdx,
                             const double w0, const double q0,
                             std::array<std::array<double, 6>, ArraySize> &coeffs) {
        if (n < 2) { return 0; }
        const size_t number = n / 2;
        const auto halfbw = std::asinh(0.5 / q0) / std::log(2);
        const auto w = w0 / std::pow(2, halfbw);
        const auto g = db_to_gain(-6 / static_cast<double>(n));
        const auto _q = g * w * w0 / std::sqrt((1 - g * g)) / (w0 * w0 - w * w);

        const auto singleCoeff = func(w0, _q);
        for (size_t i = 0; i < n / 2; ++i) {
            coeffs[i + startIdx] = singleCoeff;
        }
        return number;
    }

    template<size_t ArraySize,
        std::array<double, 4>(*lowShelfFirstOrderFunc)(double w, double g),
        std::array<double, 4>(*highShelfFirstOrderFunc)(double w, double g),
        std::array<double, 6>(*lowShelfFunc)(double w, double g, double q),
        std::array<double, 6>(*highShelfFunc)(double w, double g, double q)>
    size_t updateBandShelfCoeffs(const size_t n, const size_t startIdx,
                                 const double w0, const double g0, const double q0,
                                 std::array<std::array<double, 6>, ArraySize> &coeffs) {
        if (n < 2) { return 0; }
        const auto halfbw = std::asinh(0.5 / q0) / std::log(2);
        const auto scale = std::pow(2, halfbw);
        const auto w1 = w0 / scale;
        const auto w2 = w0 * scale;
        const auto f1 = w1 > 10.0 * 2 * pi / 48000, f2 = w2 < 22000.0 * 2 * pi / 48000;
        size_t n1 = 1;
        size_t n2 = 0;
        if (f1 && f2) {
            n1 = updateShelfCoeffs<ArraySize, lowShelfFirstOrderFunc, lowShelfFunc>(
                n, startIdx, w1, 1 / g0, std::sqrt(2) / 2, coeffs);
            n2 = updateShelfCoeffs<ArraySize, lowShelfFirstOrderFunc, lowShelfFunc>(
                n, startIdx + n1, w2, g0, std::sqrt(2) / 2, coeffs);
        } else if (f1) {
            n1 = updateShelfCoeffs<ArraySize, highShelfFirstOrderFunc, highShelfFunc>(
                n, startIdx, w1, g0, std::sqrt(2) / 2, coeffs);
        } else if (f2) {
            n1 = updateShelfCoeffs<ArraySize, lowShelfFirstOrderFunc, lowShelfFunc>(
                n, startIdx, w2, g0, std::sqrt(2) / 2, coeffs);
        } else {
            coeffs[startIdx] = {1, 1, 1, g0, g0, g0};
        }
        return n1 + n2;
    }

    template<size_t ArraySize,
        std::array<double, 4>(*lowShelfFirstOrderFunc)(double w, double g),
        std::array<double, 4>(*highShelfFirstOrderFunc)(double w, double g),
        std::array<double, 4>(*tiltShelfFirstOrderFunc)(double w, double g),
        std::array<double, 4>(*lowPassFirstOrderFunc)(double w),
        std::array<double, 4>(*highPassFirstOrderFunc)(double w),
        std::array<double, 6>(*peakFunc)(double w, double g, double q),
        std::array<double, 6>(*lowShelfFunc)(double w, double g, double q),
        std::array<double, 6>(*highShelfFunc)(double w, double g, double q),
        std::array<double, 6>(*tiltShelfFunc)(double w, double g, double q),
        std::array<double, 6>(*lowPassFunc)(double w, double q),
        std::array<double, 6>(*highPassFunc)(double w, double q),
        std::array<double, 6>(*bandPassFunc)(double w, double q),
        std::array<double, 6>(*notchFunc)(double w, double q)>
    size_t updateCoeffs(const FilterType filterType, const size_t n,
                        const double f, const double fs, const double gDB, const double q0,
                        std::array<std::array<double, 6>, ArraySize> &coeffs) {
        const auto w0 = ppi * f / fs;
        const auto g0 = db_to_gain(gDB);
        switch (filterType) {
            case peak:
                switch (n) {
                    case 0:
                    case 1: return 0;
                    case 2: {
                        coeffs[0] = peakFunc(w0, g0, q0);
                        return 1;
                    }
                    default: {
                        return updateBandShelfCoeffs<
                            ArraySize,
                            lowShelfFirstOrderFunc, highShelfFirstOrderFunc,
                            lowShelfFunc, highShelfFunc>(n, 0, w0, g0, q0, coeffs);
                    }
                }
            case lowShelf:
                return updateShelfCoeffs<ArraySize, lowShelfFirstOrderFunc, lowShelfFunc>(
                    n, 0,
                    w0, g0, std::sqrt(q0 * std::sqrt(2)) / std::sqrt(2),
                    coeffs);
            case lowPass:
                return updatePassCoeffs<ArraySize, lowPassFirstOrderFunc, lowPassFunc>(
                    n, 0,
                    w0, q0,
                    coeffs);
            case highShelf:
                return updateShelfCoeffs<ArraySize, highShelfFirstOrderFunc, highShelfFunc>(
                    n, 0,
                    w0, g0, std::sqrt(q0 * std::sqrt(2)) / std::sqrt(2),
                    coeffs);
            case highPass:
                return updatePassCoeffs<ArraySize, highPassFirstOrderFunc, highPassFunc>(
                    n, 0,
                    w0, q0,
                    coeffs);
            case bandShelf:
                return updateBandShelfCoeffs<ArraySize,
                    lowShelfFirstOrderFunc, highShelfFirstOrderFunc,
                    lowShelfFunc, highShelfFunc>(n, 0, w0, g0, q0, coeffs);
            case tiltShelf:
                return updateShelfCoeffs<ArraySize, tiltShelfFirstOrderFunc, tiltShelfFunc>(
                    n, 0,
                    w0, g0, std::sqrt(q0 * std::sqrt(2)) / std::sqrt(2),
                    coeffs);
            case notch:
                return updateNotchCoeffs<ArraySize, notchFunc>(n, 0, w0, q0, coeffs);
            case bandPass:
                return updateBandPassCoeffs<ArraySize, bandPassFunc>(n, 0, w0, q0, coeffs);
            default:
                return 0;
        }
    }
}

#endif //FILTER_DESIGN_HPP
