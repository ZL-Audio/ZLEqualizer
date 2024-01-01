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
                return getPeak(w0, g, q);
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
                return {coeff33{}};
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
        if (n < 2) {
            return {};
        }
        n = n + 2;
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
        size_t number = n / 2;
        auto theta0 = pi / static_cast<double>(number) / 4;
        auto qBase = std::pow(std::sqrt(2.0) * q0, 1 / static_cast<double>(number));
        std::vector<double> qs(number);
        for (size_t i = 0; i < number; i++) {
            auto theta = theta0 * static_cast<double>(2 * i + 1);
            qs[i] = 1.0 / 2.0 / std::cos(theta) * qBase;
        }
        return qs;
    }
}