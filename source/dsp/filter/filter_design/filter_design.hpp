// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../helpers.hpp"

namespace zldsp::filter::FilterDesign {
    template <class Coeff, FilterType filter_type>
    size_t updatePassCoeffs(const size_t n, const size_t start_idx,
                            const double w0, const double q0,
                            std::span<std::array<double, 6>> coeffs) {
        if (n == 1) {
            if constexpr (filter_type == kLowPass) {
                const auto coeff = Coeff::get1LowPass(w0);
                coeffs[start_idx] = {coeff[0], coeff[1], 0.0, coeff[2], coeff[3], 0.0};
            }
            if constexpr (filter_type == kHighPass) {
                const auto coeff = Coeff::get1HighPass(w0);
                coeffs[start_idx] = {coeff[0], coeff[1], 0.0, coeff[2], coeff[3], 0.0};
            }
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
            if constexpr (filter_type == kLowPass) {
                coeffs[i + start_idx] = Coeff::get2LowPass(w0, qs);
            }
            if constexpr (filter_type == kHighPass) {
                coeffs[i + start_idx] = Coeff::get2HighPass(w0, qs);
            }
        }
        return number;
    }

    template <class Coeff, FilterType filter_type>
    size_t updateShelfCoeffs(const size_t n, const size_t start_idx,
                             const double w0, const double g0, const double q0,
                             std::span<std::array<double, 6>> coeffs) {
        if (n == 1) {
            if constexpr (filter_type == kLowShelf) {
                const auto coeff = Coeff::get1LowShelf(w0, g0);
                coeffs[start_idx] = {coeff[0], coeff[1], 0.0, coeff[2], coeff[3], 0.0};
            }
            if constexpr (filter_type == kHighShelf) {
                const auto coeff = Coeff::get1HighShelf(w0, g0);
                coeffs[start_idx] = {coeff[0], coeff[1], 0.0, coeff[2], coeff[3], 0.0};
            }
            if constexpr (filter_type == kTiltShelf) {
                const auto coeff = Coeff::get1TiltShelf(w0, g0);
                coeffs[start_idx] = {coeff[0], coeff[1], 0.0, coeff[2], coeff[3], 0.0};
            }
            return 1;
        } else if (n == 2) {
            if constexpr (filter_type == kLowShelf) {
                coeffs[start_idx] = Coeff::get2LowShelf(w0, g0, q0);
            }
            if constexpr (filter_type == kHighShelf) {
                coeffs[start_idx] = Coeff::get2HighShelf(w0, g0, q0);
            }
            if constexpr (filter_type == kTiltShelf) {
                coeffs[start_idx] = Coeff::get2TiltShelf(w0, g0, q0);
            }
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
            if constexpr (filter_type == kLowShelf) {
                coeffs[i + start_idx] = Coeff::get2LowShelf(w0, _g, _q);
            }
            if constexpr (filter_type == kHighShelf) {
                coeffs[i + start_idx] = Coeff::get2HighShelf(w0, _g, _q);
            }
            if constexpr (filter_type == kTiltShelf) {
                coeffs[i + start_idx] = Coeff::get2TiltShelf(w0, _g, _q);
            }
        }
        return number;
    }

    template <class Coeff, FilterType filter_type>
    void updateShelfGain(const size_t n, const size_t start_idx, const double g0, const std::span<double> cache,
                         std::span<std::array<double, 6>> coeffs) {
        if (n == 1) {
            if constexpr (filter_type == kLowShelf) {
                const auto coeff = Coeff::get1LowShelf(cache[0], g0);
                coeffs[start_idx] = {coeff[0], coeff[1], 0.0, coeff[2], coeff[3], 0.0};
            }
            if constexpr (filter_type == kHighShelf) {
                const auto coeff = Coeff::get1HighShelf(cache[0], g0);
                coeffs[start_idx] = {coeff[0], coeff[1], 0.0, coeff[2], coeff[3], 0.0};
            }
            if constexpr (filter_type == kTiltShelf) {
                const auto coeff = Coeff::get1TiltShelf(cache[0], g0);
                coeffs[start_idx] = {coeff[0], coeff[1], 0.0, coeff[2], coeff[3], 0.0};
            }
        } else if (n == 2) {
            if constexpr (filter_type == kLowShelf) {
                coeffs[start_idx] = Coeff::get2LowShelf(g0, cache);
            }
            if constexpr (filter_type == kHighShelf) {
                coeffs[start_idx] = Coeff::get2HighShelf(g0, cache);
            }
            if constexpr (filter_type == kTiltShelf) {
                coeffs[start_idx] = Coeff::get2TiltShelf(g0, cache);
            }
        } else {
            const size_t number = n / 2;
            const auto _g = std::pow(g0, 1.0 / static_cast<double>(number));
            for (size_t i = 0; i < number; i++) {
                if constexpr (filter_type == kLowShelf) {
                    coeffs[i + start_idx] = Coeff::get2LowShelf(cache[0], _g, cache[i + 2]);
                }
                if constexpr (filter_type == kHighShelf) {
                    coeffs[i + start_idx] = Coeff::get2HighShelf(cache[0], _g, cache[i + 2]);
                }
                if constexpr (filter_type == kTiltShelf) {
                    coeffs[i + start_idx] = Coeff::get2TiltShelf(cache[0], _g, cache[i + 2]);
                }
            }
        }
    }

    inline void updateShelfDynamicCache(const size_t n, const double w0, const double q0, std::span<double> cache) {
        const size_t number = n / 2;
        const auto theta0 = pi / static_cast<double>(number) / 4;
        const auto scale = std::pow(std::sqrt(2.0) * q0, 1 / static_cast<double>(number));
        const auto rescale_base = std::log10(std::sqrt(2.0) * q0) / std::pow(static_cast<double>(n), 1.5) * 12;
        cache[0] = w0;
        cache[1] = q0;
        for (size_t i = 0; i < number; i++) {
            const auto centered = static_cast<double>(i) - static_cast<double>(number) / 2 + 0.5;
            const auto rescale = centered * rescale_base;
            const auto theta = theta0 * static_cast<double>(2 * i + 1);
            cache[i + 2] = 1.0 / 2.0 / std::cos(theta) * scale * std::pow(2, rescale);
        }
    }

    template <class Coeff>
    size_t updateBandPassCoeffs(const size_t n, const size_t start_idx,
                                const double w0, const double q0,
                                std::span<std::array<double, 6>> coeffs) {
        if (n < 2) { return 0; }
        const size_t number = n / 2;
        const auto halfbw = std::asinh(0.5 / q0) / std::log(2);
        const auto w = w0 / std::pow(2, halfbw);
        const auto g = dbToGain(-6 / static_cast<double>(n));
        const auto _q = std::sqrt(1 - g * g) * w * w0 / g / (w0 * w0 - w * w);

        const auto single_coeff = Coeff::get2BandPass(w0, _q);
        for (size_t i = 0; i < n / 2; ++i) {
            coeffs[i + start_idx] = single_coeff;
        }
        return number;
    }

    template <class Coeff>
    size_t updateNotchCoeffs(const size_t n, const size_t start_idx,
                             const double w0, const double q0,
                             std::span<std::array<double, 6>> coeffs) {
        if (n < 2) { return 0; }
        const size_t number = n / 2;
        const auto halfbw = std::asinh(0.5 / q0) / std::log(2);
        const auto w = w0 / std::pow(2, halfbw);
        const auto g = dbToGain(-6 / static_cast<double>(n));
        const auto _q = g * w * w0 / std::sqrt((1 - g * g)) / (w0 * w0 - w * w);

        const auto single_coeff = Coeff::get2Notch(w0, _q);
        for (size_t i = 0; i < n / 2; ++i) {
            coeffs[i + start_idx] = single_coeff;
        }
        return number;
    }

    template <class Coeff>
    size_t updateBandShelfCoeffs(const size_t n, const size_t start_idx,
                                 const double w0, const double g0, const double q0,
                                 std::span<std::array<double, 6>> coeffs) {
        if (n < 2) { return 0; }
        const auto halfbw = std::asinh(0.5 / q0) / std::log(2);
        const auto scale = std::pow(2, halfbw);
        const auto w1 = w0 / scale;
        const auto w2 = w0 * scale;
        const auto f1 = w1 > 10.0 * 2 * pi / 48000, f2 = w2 < 22000.0 * 2 * pi / 48000;
        size_t n1 = 1;
        size_t n2 = 0;
        if (f1 && f2) {
            n1 = updateShelfCoeffs<Coeff, kLowShelf>(n, start_idx, w1, 1 / g0, std::sqrt(2) / 2, coeffs);
            n2 = updateShelfCoeffs<Coeff, kLowShelf>(n, start_idx + n1, w2, g0, std::sqrt(2) / 2, coeffs);
        } else if (f1) {
            n1 = updateShelfCoeffs<Coeff, kHighShelf>(n, start_idx, w1, g0, std::sqrt(2) / 2, coeffs);
        } else if (f2) {
            n1 = updateShelfCoeffs<Coeff, kLowShelf>(n, start_idx, w2, g0, std::sqrt(2) / 2, coeffs);
        } else {
            coeffs[start_idx] = {1, 1, 1, g0, g0, g0};
        }
        return n1 + n2;
    }

    template <class Coeff>
    void updateBandShelfGain(const size_t n, const double g0, const std::span<double> cache,
                             std::span<std::array<double, 6>> coeffs) {
        if (n < 2) { return; }
        const auto halfbw = std::asinh(0.5 / cache[1]) / std::log(2);
        const auto scale = std::pow(2, halfbw);
        const auto w0 = cache[0];
        const auto w1 = cache[0] / scale;
        const auto w2 = cache[0] * scale;
        const auto f1 = w1 > 10.0 * 2 * pi / 48000;
        const auto f2 = w2 < 22000.0 * 2 * pi / 48000;
        if (f1 && f2) {
            cache[0] = w1;
            updateShelfGain<Coeff, kLowShelf>(n, 0, 1 / g0, cache, coeffs);
            cache[0] = w2;
            updateShelfGain<Coeff, kLowShelf>(n, n / 2, g0, cache, coeffs);
        } else if (f1) {
            cache[0] = w1;
            updateShelfGain<Coeff, kHighShelf>(n, 0, g0, cache, coeffs);
        } else if (f2) {
            cache[0] = w2;
            updateShelfGain<Coeff, kLowShelf>(n, 0, g0, cache, coeffs);
        } else {
            coeffs[0] = {1, 1, 1, g0, g0, g0};
        }
        cache[0] = w0;
    }

    template <class Coeff>
    size_t updateCoeffs(const FilterType filterType, const size_t n,
                        const double f, const double fs, const double gDB, const double q0,
                        std::span<std::array<double, 6>> coeffs) {
        const auto w0 = ppi * f / fs;
        const auto g0 = dbToGain(gDB);
        switch (filterType) {
        case kPeak:
            switch (n) {
            case 0:
            case 1:
                return 0;
            case 2: {
                coeffs[0] = Coeff::get2Peak(w0, g0, q0);
                return 1;
            }
            default: {
                return updateBandShelfCoeffs<Coeff>(n, 0, w0, g0, q0, coeffs);
            }
            }
        case kLowShelf:
            return updateShelfCoeffs<Coeff, kLowShelf>(
                n, 0, w0, g0, std::sqrt(q0 * std::sqrt(2)) / std::sqrt(2),
                coeffs);
        case kLowPass:
            return updatePassCoeffs<Coeff, kLowPass>(n, 0, w0, q0, coeffs);
        case kHighShelf:
            return updateShelfCoeffs<Coeff, kHighShelf>(
                n, 0, w0, g0, std::sqrt(q0 * std::sqrt(2)) / std::sqrt(2),
                coeffs);
        case kHighPass:
            return updatePassCoeffs<Coeff, kHighPass>(n, 0, w0, q0, coeffs);
        case kBandShelf:
            return updateBandShelfCoeffs<Coeff>(n, 0, w0, g0, q0, coeffs);
        case kTiltShelf:
            return updateShelfCoeffs<Coeff, kTiltShelf>(
                n, 0, w0, g0, std::sqrt(q0 * std::sqrt(2)) / std::sqrt(2),
                coeffs);
        case kNotch:
            return updateNotchCoeffs<Coeff>(n, 0, w0, q0, coeffs);
        case kBandPass:
            return updateBandPassCoeffs<Coeff>(n, 0, w0, q0, coeffs);
        default:
            return 0;
        }
    }

    template <class Coeff>
    void updateCache(const FilterType filterType, const size_t n,
                     const double f, const double fs, const double q0,
                     std::span<double> cache) {
        const auto w0 = ppi * f / fs;
        switch (filterType) {
        case kPeak: {
            if (n == 2) {
                Coeff::update2PeakDynamicCache(w0, q0, cache);
            } else if (n > 2) {
                updateShelfDynamicCache(n, w0, q0, cache);
            }
            break;
        }
        case kLowShelf:
        case kHighShelf:
        case kTiltShelf: {
            if (n == 1) {
                cache[0] = w0;
            } else if (n == 2) {
                Coeff::update2TiltShelfDynamicCache(w0, q0, cache);
            } else if (n > 2) {
                updateShelfDynamicCache(n, w0, q0, cache);
            }
            break;
        }
        case kLowPass:
        case kHighPass:
        case kBandShelf:
        case kNotch:
        case kBandPass:
        default:
            return;
        }
    }

    template <class Coeff>
    void updateGain(const FilterType filterType, const size_t n, const double gDB,
                    const std::span<double> cache,
                    std::span<std::array<double, 6>> coeffs) {
        const auto g0 = dbToGain(gDB);
        switch (filterType) {
        case kPeak: {
            if (n == 2) {
                coeffs[0] = Coeff::get2Peak(g0, cache);
            } else {
                updateBandShelfGain<Coeff>(n, g0, cache, coeffs);
            }
            break;
        }
        case kLowShelf: {
            updateShelfGain<Coeff, kLowShelf>(n, 0, g0, cache, coeffs);
            break;
        }
        case kHighShelf: {
            updateShelfGain<Coeff, kHighShelf>(n, 0, g0, cache, coeffs);
            break;
        }
        case kTiltShelf: {
            updateShelfGain<Coeff, kTiltShelf>(n, 0, g0, cache, coeffs);
            break;
        }
        case kBandShelf: {
            updateBandShelfGain<Coeff>(n, g0, cache, coeffs);
            break;
        }
        case kLowPass:
        case kHighPass:
        case kNotch:
        case kBandPass:
        default:
            break;
        }
    }
}
