// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

// Reference:
// Cascading filters, Nigel Redmon
// A Simple Method of Designing Multiple Order All Pole Bandpass Filters by Cascading 2nd Order Sections, Nello Sevastopoulos and Richard Markell

#include "design_filter.hpp"

constexpr static double pi = std::numbers::pi;
constexpr static double ppi = 2 * std::numbers::pi;

namespace zlFilter {
    size_t DesignFilter::updateCoeff(const FilterType filterType,
                                     const double f, const double fs,
                                     const double gDB, const double q,
                                     const size_t n, std::array<coeff33, 16> &coeffs) {
        auto w0 = ppi * f / fs;
        auto g = db_to_gain(gDB);
        switch (filterType) {
            case peak:
                switch (n) {
                    case 0:
                    case 1: return 0;
                    case 2: return updatePeak(w0, g, q, coeffs);
                    default: return updateBandShelf(n, w0, g, q, coeffs, 0);
                }
            case lowShelf:
                return updateLowShelf(n, w0, g, std::sqrt(q * std::sqrt(2)) / std::sqrt(2), coeffs, 0);
            case lowPass:
                return updateLowPass(n, w0, q, coeffs, 0);
            case highShelf:
                return updateHighShelf(n, w0, g, std::sqrt(q * std::sqrt(2)) / std::sqrt(2), coeffs, 0);
            case highPass:
                return updateHighPass(n, w0, q, coeffs, 0);
            case bandShelf:
                return updateBandShelf(n, w0, g, q, coeffs, 0);
            case tiltShelf:
                return updateTiltShelf(n, w0, g, std::sqrt(q * std::sqrt(2)) / std::sqrt(2), coeffs, 0);
            case notch:
                return updateNotch(n, w0, q, coeffs, 0);
            case bandPass:
                return updateBandPass(n, w0, q, coeffs, 0);
            default:
                return 0;
        }
    }

    size_t DesignFilter::updateLowPass(size_t n, double w0, double q, std::array<coeff33, 16> &coeffs,
                                       size_t startIdx) {
        if (n == 1) {
            auto [a, b] = MartinCoeff::get1LowPass(w0);
            coeffs[0] = {{a[0], a[1], 0.0}, {b[0], b[1], 0.0}};
            return 1;
        }
        const size_t number = n / 2;
        const auto theta0 = pi / static_cast<double>(number) / 4;
        const auto scale = std::pow(std::sqrt(2.0) * q, 1 / static_cast<double>(number));
        const auto rescale_base = std::log10(std::sqrt(2.0) * q) / std::pow(static_cast<double>(n), 1.5) * 12;
        for (size_t i = 0; i < number; i++) {
            const auto centered = static_cast<double>(i) - static_cast<double>(number) / 2 + 0.5;
            const auto rescale = centered * rescale_base;
            const auto theta = theta0 * static_cast<double>(2 * i + 1);
            const auto qs = 1.0 / 2.0 / std::cos(theta) * scale * std::pow(2, rescale);
            coeffs[i + startIdx] = MartinCoeff::get2LowPass(w0, qs);
        }
        return number;
    }

    size_t DesignFilter::updateHighPass(size_t n, double w0, double q, std::array<coeff33, 16> &coeffs,
                                        size_t startIdx) {
        if (n == 1) {
            auto [a, b] = MartinCoeff::get1HighPass(w0);
            coeffs[0] = {{a[0], a[1], 0.0}, {b[0], b[1], 0.0}};
            return 1;
        }
        const size_t number = n / 2;
        const auto theta0 = pi / static_cast<double>(number) / 4;
        const auto scale = std::pow(std::sqrt(2.0) * q, 1 / static_cast<double>(number));
        const auto rescale_base = std::log10(std::sqrt(2.0) * q) / std::pow(static_cast<double>(n), 1.5) * 12;
        for (size_t i = 0; i < number; i++) {
            const auto centered = static_cast<double>(i) - static_cast<double>(number) / 2 + 0.5;
            const auto rescale = centered * rescale_base;
            const auto theta = theta0 * static_cast<double>(2 * i + 1);
            const auto qs = 1.0 / 2.0 / std::cos(theta) * scale * std::pow(2, rescale);
            coeffs[i + startIdx] = MartinCoeff::get2HighPass(w0, qs);
        }
        return number;
    }

    size_t DesignFilter::updateTiltShelf(size_t n, double w0, double g, double q, std::array<coeff33, 16> &coeffs,
                                         size_t startIdx) {
        if (n == 1) {
            auto [a, b] = MartinCoeff::get1TiltShelf(w0, g);
            coeffs[0] = {{a[0], a[1], 0.0}, {b[0], b[1], 0.0}};
            return 1;
        }
        const size_t number = n / 2;
        const auto _g = std::pow(g, 1.0 / static_cast<double>(number));
        const auto theta0 = pi / static_cast<double>(number) / 4;
        const auto scale = std::pow(std::sqrt(2.0) * q, 1 / static_cast<double>(number));
        const auto rescale_base = std::log10(std::sqrt(2.0) * q) / std::pow(static_cast<double>(n), 1.5) * 12;
        for (size_t i = 0; i < number; i++) {
            const auto centered = static_cast<double>(i) - static_cast<double>(number) / 2 + 0.5;
            const auto rescale = centered * rescale_base;
            const auto theta = theta0 * static_cast<double>(2 * i + 1);
            const auto _q = 1.0 / 2.0 / std::cos(theta) * scale * std::pow(2, rescale);
            coeffs[i + startIdx] = MartinCoeff::get2TiltShelf(w0, _g, _q);
        }
        return number;
    }

    size_t DesignFilter::updateLowShelf(size_t n, double w0, double g, double q, std::array<coeff33, 16> &coeffs,
                                        size_t startIdx) {
        if (n == 1) {
            auto [a, b] = MartinCoeff::get1LowShelf(w0, g);
            coeffs[0] = {{a[0], a[1], 0.0}, {b[0], b[1], 0.0}};
            return 1;
        }
        const size_t number = n / 2;
        const auto _g = std::pow(g, 1.0 / static_cast<double>(number));
        const auto theta0 = pi / static_cast<double>(number) / 4;
        const auto scale = std::pow(std::sqrt(2.0) * q, 1 / static_cast<double>(number));
        const auto rescale_base = std::log10(std::sqrt(2.0) * q) / std::pow(static_cast<double>(n), 1.5) * 12;
        for (size_t i = 0; i < number; i++) {
            const auto centered = static_cast<double>(i) - static_cast<double>(number) / 2 + 0.5;
            const auto rescale = centered * rescale_base;
            const auto theta = theta0 * static_cast<double>(2 * i + 1);
            const auto _q = 1.0 / 2.0 / std::cos(theta) * scale * std::pow(2, rescale);
            coeffs[i + startIdx] = MartinCoeff::get2LowShelf(w0, _g, _q);
        }
        return number;
    }

    size_t DesignFilter::updateHighShelf(size_t n, double w0, double g, double q, std::array<coeff33, 16> &coeffs,
                                         size_t startIdx) {
        if (n == 1) {
            auto [a, b] = MartinCoeff::get1HighShelf(w0, g);
            coeffs[0] = {{a[0], a[1], 0.0}, {b[0], b[1], 0.0}};
            return 1;
        }
        const size_t number = n / 2;
        const auto _g = std::pow(g, 1.0 / static_cast<double>(number));
        const auto theta0 = pi / static_cast<double>(number) / 4;
        const auto scale = std::pow(std::sqrt(2.0) * q, 1 / static_cast<double>(number));
        const auto rescale_base = std::log10(std::sqrt(2.0) * q) / std::pow(static_cast<double>(n), 1.5) * 12;
        for (size_t i = 0; i < number; i++) {
            const auto centered = static_cast<double>(i) - static_cast<double>(number) / 2 + 0.5;
            const auto rescale = centered * rescale_base;
            const auto theta = theta0 * static_cast<double>(2 * i + 1);
            const auto _q = 1.0 / 2.0 / std::cos(theta) * scale * std::pow(2, rescale);
            coeffs[i + startIdx] = MartinCoeff::get2HighShelf(w0, _g, _q);
        }
        return number;
    }

    size_t DesignFilter::updateBandPass(size_t n, double w0, double q, std::array<coeff33, 16> &coeffs,
                                        size_t startIdx) {
        auto halfbw = std::asinh(0.5 / q) / std::log(2);
        auto w = w0 / std::pow(2, halfbw);
        auto g = db_to_gain(-6 / static_cast<double>(n));
        auto _q = std::sqrt(1 - g * g) * w * w0 / g / (w0 * w0 - w * w);

        _q = std::max(_q, 0.025);
        const auto singleCoeff = MartinCoeff::get2BandPass(w0, _q);
        for (size_t i = 0; i < n / 2; ++i) {
            coeffs[i + startIdx] = singleCoeff;
        }
        return n / 2;
    }

    size_t DesignFilter::updateNotch(size_t n, double w0, double q, std::array<coeff33, 16> &coeffs, size_t startIdx) {
        auto halfbw = std::asinh(0.5 / q) / std::log(2);
        auto w = w0 / std::pow(2, halfbw);
        auto g = db_to_gain(-6 / static_cast<double>(n));
        auto _q = g * w * w0 / std::sqrt((1 - g * g)) / (w0 * w0 - w * w);

        const auto singleCoeff = MartinCoeff::get2Notch(w0, _q);
        for (size_t i = 0; i < n / 2; ++i) {
            coeffs[i + startIdx] = singleCoeff;
        }
        return n / 2;
    }

    size_t DesignFilter::updatePeak(double w0, double g, double q, std::array<coeff33, 16> &coeffs) {
        coeffs[0] = MartinCoeff::get2Peak(w0, g, q);
        return 1;
    }

    size_t DesignFilter::updateBandShelf(size_t n, double w0, double g, double q, std::array<coeff33, 16> &coeffs,
                                         size_t startIdx) {
        if (n <= 2) {
            return 0;
        }
        const auto halfbw = std::asinh(0.5 / q) / std::log(2);
        const auto scale = std::pow(2, halfbw);
        const auto w1 = w0 / scale;
        const auto w2 = w0 * scale;
        const auto f1 = w1 > 10.0 * 2 * pi / 48000, f2 = w2 < 22000.0 * 2 * pi / 48000;
        size_t n1 = 1;
        size_t n2 = 0;
        if (f1 && f2) {
            n1 = updateLowShelf(n, w1, 1 / g, std::sqrt(2) / 2, coeffs, startIdx);
            n2 = updateLowShelf(n, w2, g, std::sqrt(2) / 2, coeffs, startIdx + n1);
        } else if (f1) {
            n1 = updateHighShelf(n, w1, g, std::sqrt(2) / 2, coeffs, startIdx);
        } else if (f2) {
            n1 = updateLowShelf(n, w2, g, std::sqrt(2) / 2, coeffs, startIdx);
        } else {
            coeffs[startIdx] = {{1, 1, 1}, {g, g, g}};
        }
        return n1 + n2;
    }
}
