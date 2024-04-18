// Copyright (C) 2023 - zsliu98
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

const static double pi = std::numbers::pi;
const static double ppi = 2 * std::numbers::pi;

namespace zlIIR {
    std::vector<coeff33> DesignFilter::getCoeff(zlIIR::FilterType filterType,
                                                double f, double fs, double gDB, double q, size_t n) {
        auto w0 = ppi * f / fs;
        auto g = db_to_gain(gDB);
        switch (filterType) {
            case peak:
                switch (n) {
                    case 0:
                    case 1: return {coeff33{{1, 1, 1}, {1, 1, 1}}};
                    case 2: return getPeak(w0, g, q);
                    default: return getBandShelf(n, w0, g, q);
                }
            case lowShelf:
                return getLowShelf(n, w0, g, std::sqrt(q * std::sqrt(2)) / std::sqrt(2));
            case lowPass:
                return getLowPass(n, w0, q);
            case highShelf:
                return getHighShelf(n, w0, g, std::sqrt(q * std::sqrt(2)) / std::sqrt(2));
            case highPass:
                return getHighPass(n, w0, q);
            case bandShelf:
                return getBandShelf(n, w0, g, q);
            case tiltShelf:
                return getTiltShelf(n, w0, g, std::sqrt(q * std::sqrt(2)) / std::sqrt(2));
            case notch:
                return getNotch(n, w0, q);
            case bandPass:
                return getBandPass(n, w0, q);
            default:
                return {coeff33{{1, 1, 1}, {1, 1, 1}}};
        }
    }

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


    std::vector<coeff33> DesignFilter::getLowPass(size_t n, double w0, double q) {
        if (n == 1) {
            auto [a, b] = MartinCoeff::get1LowPass(w0);
            return {{{a[0], a[1], 0.0}, {b[0], b[1], 0.0}}};
        } else {
            auto qs = getQs(n, q);
            std::vector<coeff33> coeff;
            coeff.reserve(qs.size());
            for (auto _q: qs) {
                coeff.push_back(MartinCoeff::get2LowPass(w0, _q));
            }
            return coeff;
        }
    }

    std::vector<coeff33> DesignFilter::getHighPass(size_t n, double w0, double q) {
        if (n == 1) {
            auto [a, b] = MartinCoeff::get1HighPass(w0);
            return {{{a[0], a[1], 0.0}, {b[0], b[1], 0.0}}};
        } else {
            auto qs = getQs(n, q);
            std::vector<coeff33> coeff;
            coeff.reserve(qs.size());
            for (auto _q: qs) {
                coeff.push_back(MartinCoeff::get2HighPass(w0, _q));
            }
            return coeff;
        }
    }

    std::vector<coeff33> DesignFilter::getTiltShelf(size_t n, double w0, double g, double q) {
        if (n == 1) {
            auto [a, b] = MartinCoeff::get1TiltShelf(w0, g);
            return {{{a[0], a[1], 0.0}, {b[0], b[1], 0.0}}};
        } else {
            auto qs = getQs(n, q);
            auto _g = std::pow(g, 1.0 / static_cast<double>(qs.size()));
            std::vector<coeff33> coeff;
            coeff.reserve(qs.size());
            for (auto _q: qs) {
                coeff.push_back(MartinCoeff::get2TiltShelf(w0, _g, _q));
            }
            return coeff;
        }
    }

    std::vector<coeff33> DesignFilter::getLowShelf(size_t n, double w0, double g, double q) {
        if (n == 1) {
            auto [a, b] = MartinCoeff::get1LowShelf(w0, g);
            return {{{a[0], a[1], 0.0}, {b[0], b[1], 0.0}}};
        } else {
            auto qs = getQs(n, q);
            auto _g = std::pow(g, 1.0 / static_cast<double>(qs.size()));
            std::vector<coeff33> coeff;
            coeff.reserve(qs.size());
            for (auto _q: qs) {
                coeff.push_back(MartinCoeff::get2LowShelf(w0, _g, _q));
            }
            return coeff;
        }
    }

    std::vector<coeff33> DesignFilter::getHighShelf(size_t n, double w0, double g, double q) {
        if (n == 1) {
            auto [a, b] = MartinCoeff::get1HighShelf(w0, g);
            return {{{a[0], a[1], 0.0}, {b[0], b[1], 0.0}}};
        } else {
            auto qs = getQs(n, q);
            auto _g = std::pow(g, 1.0 / static_cast<double>(qs.size()));
            std::vector<coeff33> coeff;
            coeff.reserve(qs.size());
            for (auto _q: qs) {
                coeff.push_back(MartinCoeff::get2HighShelf(w0, _g, _q));
            }
            return coeff;
        }
    }

    std::vector<coeff33> DesignFilter::getBandPass(size_t n, double w0, double q) {
        auto bw = 2 * std::asinh(0.5 / q) / std::log(2);
        auto w = w0 / std::pow(2, bw / 2);
        auto g = db_to_gain(-6 / static_cast<double>(n));
        auto _q = std::sqrt(1 - g * g) * w * w0 / g / (w0 * w0 - w * w);

        std::vector<coeff33> coeff;
        coeff.reserve(n / 2);
        _q = std::max(_q, 0.025);
        auto singleCoeff = MartinCoeff::get2BandPass(w0, _q);
        for (size_t i = 0; i < n / 2; ++i) {
            coeff.push_back(singleCoeff);
        }
        return coeff;
    }

    std::vector<coeff33> DesignFilter::getNotch(size_t n, double w0, double q) {
        auto bw = 2 * std::asinh(0.5 / q) / std::log(2);
        auto w = w0 / std::pow(2, bw / 2);
        auto g = db_to_gain(-6 / static_cast<double>(n));
        auto _q = g * w * w0 / std::sqrt((1 - g * g)) / (w0 * w0 - w * w);

        std::vector<coeff33> coeff;
        coeff.reserve(n / 2);
        auto singleCoeff = MartinCoeff::get2Notch(w0, _q);
        for (size_t i = 0; i < n / 2; ++i) {
            coeff.push_back(singleCoeff);
        }
        return coeff;
    }

    std::vector<coeff33> DesignFilter::getPeak(double w0, double g, double q) {
        return {MartinCoeff::get2Peak(w0, g, q)};
    }

    std::vector<coeff33> DesignFilter::getBandShelf(size_t n, double w0, double g, double q) {
        if (n <= 2) {
            return {};
        }
        auto bw = 2 * std::asinh(0.5 / q) / std::log(2);
        auto w1 = w0 / std::pow(2, bw / 2);
        auto w2 = w0 * std::pow(2, bw / 2);
        std::vector<coeff33> coeff1;
        if (w1 > 10.0 * 2 * pi / 48000) {
            coeff1 = getLowShelf(n, w1, 1 / g, std::sqrt(2) / 2);
        } else {
            coeff1.push_back({{1, 1, 1}, {1, 1, 1}});
        }
        if (w2 < 22000.0 * 2 * pi / 48000) {
            auto coeff2 = getLowShelf(n, w2, g, std::sqrt(2) / 2);
            coeff1.insert(coeff1.end(), coeff2.begin(), coeff2.end());
        } else {
            coeff1.push_back({{1, 1, 1}, {g, g, g}});
        }
        return coeff1;
    }

    std::vector<double> DesignFilter::getQs(size_t n, double q0) {
        const size_t number = n / 2;
        const auto theta0 = pi / static_cast<double>(number) / 4;
        const auto scale = std::pow(std::sqrt(2.0) * q0, 1 / static_cast<double>(number));
        const auto rescale_base = std::log10(std::sqrt(2.0) * q0) / std::pow(static_cast<double>(n), 1.5) * 12;
        std::vector<double> qs(number);
        for (size_t i = 0; i < number; i++) {
            const auto centered = static_cast<double>(i) - static_cast<double>(number) / 2 + 0.5;
            const auto rescale = centered * rescale_base;
            const auto theta = theta0 * static_cast<double>(2 * i + 1);
            qs[i] = 1.0 / 2.0 / std::cos(theta) * scale * std::pow(2, rescale);
        }
        return qs;
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

    size_t DesignFilter::updateBandPass(size_t n, double w0, double q, std::array<coeff33, 16> &coeffs, size_t startIdx) {
        auto bw = 2 * std::asinh(0.5 / q) / std::log(2);
        auto w = w0 / std::pow(2, bw / 2);
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
        auto bw = 2 * std::asinh(0.5 / q) / std::log(2);
        auto w = w0 / std::pow(2, bw / 2);
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

    size_t DesignFilter::updateBandShelf(size_t n, double w0, double g, double q, std::array<coeff33, 16> &coeffs, size_t startIdx ) {
        if (n <= 2) {
            return 0;
        }
        const auto bw = 2 * std::asinh(0.5 / q) / std::log(2);
        const auto w1 = w0 / std::pow(2, bw / 2);
        const auto w2 = w0 * std::pow(2, bw / 2);
        size_t n1 = 0;
        if (w1 > 10.0 * 2 * pi / 48000) {
            n1 = updateLowShelf(n, w1, 1 / g, std::sqrt(2) / 2, coeffs, startIdx);
        }
        size_t n2 = 0;
        if (w2 < 22000.0 * 2 * pi / 48000) {
            n2 = updateLowShelf(n, w2, g, std::sqrt(2) / 2, coeffs, startIdx + n1);
        } else {
            n2 = 1;
            coeffs[startIdx + n1] = {{1, 1, 1}, {g, g, g}};
        }
        return n1 + n2;
    }
}
