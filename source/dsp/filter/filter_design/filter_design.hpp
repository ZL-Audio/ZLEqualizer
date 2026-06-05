// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include "../helpers.hpp"

namespace zldsp::filter::FilterDesign {
    static constexpr std::array<double, 9> flat_freq = {
        10.0, 40.0, 160.0, 640.0, 2560.0, 10240.0, 40960.0, 163840.0, 655360.0
    };

    static constexpr std::array<double, 9> inv_flat_freq_sqr = []() {
        std::array<double, flat_freq.size()> arr{};
        for (size_t i = 0; i < flat_freq.size(); ++i) {
            arr[i] = 1.0 / (flat_freq[i] * flat_freq[i]);
        }
        return arr;
    }();

    template <class Coeff>
    size_t updateFlatShelfCoeffs(const double freq, const double fs, const double g_dB,
                                std::span<std::array<double, 5>> coeffs) {
        size_t num_filters;
        if (fs < 50000.0) {
            num_filters = 6;
        } else if (fs < 200000.0) {
            num_filters = 7;
        } else if (fs < 800000.0) {
            num_filters = 8;
        } else {
            num_filters = 9;
        }

        const auto g_shelf_dB = g_dB * 0.5;
        const double g_linear = dbToGain(g_shelf_dB);
        const double g_linear_sq = g_linear * g_linear;
        const double freq_sq = freq * freq;

        double total_mag_sq = 1.0;
        for (size_t i = 0; i < num_filters; ++i) {
            const double w_sq = freq_sq * inv_flat_freq_sqr[i];
            total_mag_sq *= (g_linear_sq * w_sq + g_linear) / (w_sq + g_linear);
        }
        const double makeup_gain = 1.0 / std::sqrt(total_mag_sq);

        const double inv_fs = 1.0 / fs;
        for (size_t i = 0; i < num_filters - 1; ++i) {
            const double w_i = flat_freq[i] * inv_fs;
            const auto coeff = Coeff::get1HighShelf(w_i, g_shelf_dB);
            coeffs[i] = {coeff[0], 0.0, coeff[1], coeff[2], 0.0};
        }
        {
            const auto i = num_filters - 1;
            const double w_i = flat_freq[i] * inv_fs;
            const auto coeff = Coeff::get1HighShelf(w_i, g_shelf_dB);
            coeffs[i] = {coeff[0], 0.0, coeff[1] * makeup_gain, coeff[2] * makeup_gain, 0.0};
        }

        return num_filters;
    }

    template <class Coeff>
    inline void updateFlatShelfDynamicCache(const double f, const double fs, double* cache) {
        size_t num_filters;
        if (fs < 50000.0) {
            num_filters = 6;
        } else if (fs < 200000.0) {
            num_filters = 7;
        } else if (fs < 800000.0) {
            num_filters = 8;
        } else {
            num_filters = 9;
        }
        cache[0] = static_cast<double>(num_filters);

        const double freq_sq = f * f;
        for (size_t i = 0; i < num_filters; ++i) {
            cache[i + 1] = freq_sq * inv_flat_freq_sqr[i];
        }

        const double inv_fs = 1.0 / fs;
        for (size_t i = 0; i < num_filters; ++i) {
            const double wi = flat_freq[i] * inv_fs;
            Coeff::update1ShelfDynamicCache(wi, cache + i + 1 + num_filters);
        }
    }

    template <class Coeff, FilterType filter_type>
    size_t updatePassCoeffs(const size_t n, const size_t start_idx,
                            const double w0, const double q0,
                            std::span<std::array<double, 5>> coeffs) {
        if (n == 1) {
            if constexpr (filter_type == kLowPass) {
                const auto coeff = Coeff::get1LowPass(w0);
                coeffs[start_idx] = {coeff[0], 0.0, coeff[1], coeff[2], 0.0};
            }
            if constexpr (filter_type == kHighPass) {
                const auto coeff = Coeff::get1HighPass(w0);
                coeffs[start_idx] = {coeff[0], 0.0, coeff[1], coeff[2], 0.0};
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
                             const double w0, const double g_dB, const double q0,
                             std::span<std::array<double, 5>> coeffs) {
        if (n == 1) {
            if constexpr (filter_type == kLowShelf) {
                const auto coeff = Coeff::get1LowShelf(w0, g_dB);
                coeffs[start_idx] = {coeff[0], 0.0, coeff[1], coeff[2], 0.0};
            }
            if constexpr (filter_type == kHighShelf) {
                const auto coeff = Coeff::get1HighShelf(w0, g_dB);
                coeffs[start_idx] = {coeff[0], 0.0, coeff[1], coeff[2], 0.0};
            }
            if constexpr (filter_type == kTiltShelf) {
                const auto coeff = Coeff::get1TiltShelf(w0, g_dB);
                coeffs[start_idx] = {coeff[0], 0.0, coeff[1], coeff[2], 0.0};
            }
            return 1;
        } else if (n == 2) {
            if constexpr (filter_type == kLowShelf) {
                coeffs[start_idx] = Coeff::get2LowShelf(w0, g_dB, q0);
            }
            if constexpr (filter_type == kHighShelf) {
                coeffs[start_idx] = Coeff::get2HighShelf(w0, g_dB, q0);
            }
            if constexpr (filter_type == kTiltShelf) {
                coeffs[start_idx] = Coeff::get2TiltShelf(w0, g_dB, q0);
            }
            return 1;
        }
        const size_t number = n / 2;
        const auto _g = g_dB / static_cast<double>(number);
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

    template <class Coeff>
    inline void updateShelfDynamicCache(const size_t n, const double w0, const double q0, double* cache) {
        const size_t number = n / 2;
        const auto theta0 = pi / static_cast<double>(number) / 4;
        const auto scale = std::pow(std::sqrt(2.0) * q0, 1 / static_cast<double>(number));
        const auto rescale_base = std::log10(std::sqrt(2.0) * q0) / std::pow(static_cast<double>(n), 1.5) * 12;
        for (size_t i = 0; i < number; i++) {
            const auto centered = static_cast<double>(i) - static_cast<double>(number) / 2 + 0.5;
            const auto rescale = centered * rescale_base;
            const auto theta = theta0 * static_cast<double>(2 * i + 1);
            const auto _q = 1.0 / 2.0 / std::cos(theta) * scale * std::pow(2, rescale);
            Coeff::update2ShelfDynamicCache(w0, _q, cache + (i * 3));
        }
    }

    template <class Coeff>
    size_t updateBandPassCoeffs(const size_t n, const size_t start_idx,
                                const double w0, const double q0,
                                std::span<std::array<double, 5>> coeffs) {
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
                             std::span<std::array<double, 5>> coeffs) {
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
                                 const double w0, const double g_dB, const double q0,
                                 std::span<std::array<double, 5>> coeffs) {
        if (n < 2) { return 0; }
        const auto halfbw = std::asinh(0.5 / q0) / std::log(2);
        const auto scale = std::pow(2, halfbw);
        const auto w1 = w0 / scale;
        const auto w2 = w0 * scale;
        const auto f1 = w1 > 10.0 / 48000.0, f2 = w2 < 22000.0 / 48000.0;
        size_t n1 = 1;
        size_t n2 = 0;
        if (f1 && f2) {
            n1 = updateShelfCoeffs<Coeff, kLowShelf>(n, start_idx, w1, -g_dB, std::sqrt(2) / 2, coeffs);
            n2 = updateShelfCoeffs<Coeff, kLowShelf>(n, start_idx + n1, w2, g_dB, std::sqrt(2) / 2, coeffs);
        } else if (f1) {
            n1 = updateShelfCoeffs<Coeff, kHighShelf>(n, start_idx, w1, g_dB, std::sqrt(2) / 2, coeffs);
        } else if (f2) {
            n1 = updateShelfCoeffs<Coeff, kLowShelf>(n, start_idx, w2, g_dB, std::sqrt(2) / 2, coeffs);
        } else {
            const auto g_linear = std::exp2(g_dB * kDbToExp2);
            coeffs[start_idx] = {1.0, 1.0, g_linear, g_linear, g_linear};
        }
        return n1 + n2;
    }

    template <class Coeff>
    inline void updateBandShelfDynamicCache(const size_t n, const double w0, const double q0, double* cache) {
        if (n < 2) {
            return;
        }
        const auto halfbw = std::asinh(0.5 / q0) / std::log(2);
        const auto scale = std::pow(2, halfbw);
        const auto w1 = w0 / scale;
        const auto w2 = w0 * scale;
        const auto f1 = w1 > 10.0 / 48000.0;
        const auto f2 = w2 < 22000.0 / 48000.0;
        const size_t number = n / 2;
        if (f1 && f2) {
            cache[0] = 3.0;
            updateShelfDynamicCache<Coeff>(n, w1, std::sqrt(2.0) / 2.0, cache + 1);
            updateShelfDynamicCache<Coeff>(n, w2, std::sqrt(2.0) / 2.0, cache + 1 + 3 * number);
        } else if (f1) {
            cache[0] = 1.0;
            updateShelfDynamicCache<Coeff>(n, w1, std::sqrt(2.0) / 2.0, cache + 1);
        } else if (f2) {
            cache[0] = 2.0;
            updateShelfDynamicCache<Coeff>(n, w2, std::sqrt(2.0) / 2.0, cache + 1);
        } else {
            cache[0] = 0.0;
        }
    }

    template <class Coeff>
    size_t updateCoeffs(const FilterType filterType, const size_t n,
                        const double f, const double fs, const double g_dB, const double q0,
                        std::span<std::array<double, 5>> coeffs) {
        const auto w0 = f / fs;
        const auto g0 = g_dB;
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
                n, 0, w0, g0, std::sqrt(q0 * std::sqrt(2)) / std::sqrt(2), coeffs);
        case kLowPass:
            return updatePassCoeffs<Coeff, kLowPass>(n, 0, w0, q0, coeffs);
        case kHighShelf:
            return updateShelfCoeffs<Coeff, kHighShelf>(
                n, 0, w0, g0, std::sqrt(q0 * std::sqrt(2)) / std::sqrt(2), coeffs);
        case kHighPass:
            return updatePassCoeffs<Coeff, kHighPass>(n, 0, w0, q0, coeffs);
        case kTiltShelf:
            return updateShelfCoeffs<Coeff, kTiltShelf>(
                n, 0, w0, g0, std::sqrt(q0 * std::sqrt(2)) / std::sqrt(2), coeffs);
        case kFlatTilt:
            return updateFlatShelfCoeffs<Coeff>(f, fs, g_dB, coeffs);
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
                     double* cache) {
        const auto w0 = f / fs;
        switch (filterType) {
        case kPeak: {
            if (n == 2) {
                Coeff::update2PeakDynamicCache(w0, q0, cache);
            } else if (n > 2) {
                updateBandShelfDynamicCache<Coeff>(n, w0, q0, cache);
            }
            break;
        }
        case kLowShelf:
        case kHighShelf:
        case kTiltShelf: {
            if (n == 1) {
                Coeff::update1ShelfDynamicCache(w0, cache);
            } else if (n == 2) {
                const auto q_modified = std::sqrt(q0 * std::sqrt(2.0)) / std::sqrt(2.0);
                Coeff::update2ShelfDynamicCache(w0, q_modified, cache);
            } else if (n > 2) {
                const auto q_modified = std::sqrt(q0 * std::sqrt(2.0)) / std::sqrt(2.0);
                updateShelfDynamicCache<Coeff>(n, w0, q_modified, cache);
            }
            break;
        }
        case kFlatTilt: {
            updateFlatShelfDynamicCache<Coeff>(f, fs, cache);
        }
        case kLowPass:
        case kHighPass:
        case kNotch:
        case kBandPass:
        default:
            return;
        }
    }

    template <class Coeff, FilterType filter_type>
    void updateShelfGainLinear(const size_t n, const size_t start_idx, const double g_linear_sqrt, const double* cache,
                               std::span<std::array<double, 5>> coeffs) {
        if (n == 1) {
            if constexpr (filter_type == kLowShelf) {
                const auto coeff = Coeff::get1LowShelfWithCache(g_linear_sqrt, cache);
                coeffs[start_idx] = {coeff[0], 0.0, coeff[1], coeff[2], 0.0};
            }
            if constexpr (filter_type == kHighShelf) {
                const auto coeff = Coeff::get1HighShelfWithCache(g_linear_sqrt, cache);
                coeffs[start_idx] = {coeff[0], 0.0, coeff[1], coeff[2], 0.0};
            }
            if constexpr (filter_type == kTiltShelf) {
                const auto coeff = Coeff::get1TiltShelfWithCache(g_linear_sqrt, cache);
                coeffs[start_idx] = {coeff[0], 0.0, coeff[1], coeff[2], 0.0};
            }
        } else if (n == 2) {
            if constexpr (filter_type == kLowShelf) {
                coeffs[start_idx] = Coeff::get2LowShelfWithCache(g_linear_sqrt, cache);
            }
            if constexpr (filter_type == kHighShelf) {
                coeffs[start_idx] = Coeff::get2HighShelfWithCache(g_linear_sqrt, cache);
            }
            if constexpr (filter_type == kTiltShelf) {
                coeffs[start_idx] = Coeff::get2TiltShelfWithCache(g_linear_sqrt, cache);
            }
        } else {
            const size_t number = n / 2;
            for (size_t i = 0; i < number; i++) {
                if constexpr (filter_type == kLowShelf) {
                    coeffs[i + start_idx] = Coeff::get2LowShelfWithCache(g_linear_sqrt, cache + (i * 3));
                }
                if constexpr (filter_type == kHighShelf) {
                    coeffs[i + start_idx] = Coeff::get2HighShelfWithCache(g_linear_sqrt, cache + (i * 3));
                }
                if constexpr (filter_type == kTiltShelf) {
                    coeffs[i + start_idx] = Coeff::get2TiltShelfWithCache(g_linear_sqrt, cache + (i * 3));
                }
            }
        }
    }

    template <class Coeff>
    void updateBandShelfGainLinear(const size_t n, const double g_linear_sqrt, const double* cache,
                                   std::span<std::array<double, 5>> coeffs) {
        const auto mode = static_cast<int>(cache[0]);
        if (mode == 3) {
            const size_t number = n / 2;
            updateShelfGainLinear<Coeff, kLowShelf>(n, 0, 1.0 / g_linear_sqrt, cache + 1, coeffs);
            updateShelfGainLinear<Coeff, kLowShelf>(n, number, g_linear_sqrt, cache + 1 + 3 * number, coeffs);
        } else if (mode == 1) {
            updateShelfGainLinear<Coeff, kHighShelf>(n, 0, g_linear_sqrt, cache + 1, coeffs);
        } else if (mode == 2) {
            updateShelfGainLinear<Coeff, kLowShelf>(n, 0, g_linear_sqrt, cache + 1, coeffs);
        } else {
            const auto g_linear = g_linear_sqrt * g_linear_sqrt;
            coeffs[0] = {1.0, 1.0, g_linear, g_linear, g_linear};
        }
    }

    template <class Coeff>
    void updateFlatShelfGainLinear(const double g_linear_sqrt, const double* cache,
                                  std::span<std::array<double, 5>> coeffs) {
        const auto num_filters = static_cast<size_t>(cache[0]);
        const auto g_shelf_linear = g_linear_sqrt;
        const auto g_shelf_linear_sq = g_shelf_linear * g_shelf_linear;
        double total_mag_sq = 1.0;
        for (size_t i = 0; i < num_filters; ++i) {
            const double w_sq = cache[i + 1];
            total_mag_sq *= (g_shelf_linear_sq * w_sq + g_shelf_linear) / (w_sq + g_shelf_linear);
        }
        const double makeup_gain = 1.0 / std::sqrt(total_mag_sq);

        const double* cache_shift = cache + 1 + num_filters;
        const auto g_shelf_linear_sqrt = std::sqrt(g_shelf_linear);
        for (size_t i = 0; i < num_filters - 1; ++i) {
            const auto coeff = Coeff::get1HighShelfWithCache(g_shelf_linear_sqrt, cache_shift + i);
            coeffs[i] = {coeff[0], 0.0, coeff[1], coeff[2], 0.0};
        }
        {
            const size_t i = num_filters - 1;
            const auto coeff = Coeff::get1HighShelfWithCache(g_shelf_linear_sqrt, cache_shift + i);
            coeffs[i] = {coeff[0], 0.0, coeff[1] * makeup_gain, coeff[2] * makeup_gain, 0.0};
        }
    }

    template <class Coeff>
    void updateGainLinear(const FilterType filterType, const size_t n, const double g_linear_sqrt,
                          const double* cache, std::span<std::array<double, 5>> coeffs) {
        switch (filterType) {
        case kPeak: {
            if (n == 2) {
                coeffs[0] = Coeff::get2PeakWithCache(g_linear_sqrt, cache);
            } else {
                updateBandShelfGainLinear<Coeff>(n, g_linear_sqrt, cache, coeffs);
            }
            break;
        }
        case kLowShelf: {
            updateShelfGainLinear<Coeff, kLowShelf>(n, 0, g_linear_sqrt, cache, coeffs);
            break;
        }
        case kHighShelf: {
            updateShelfGainLinear<Coeff, kHighShelf>(n, 0, g_linear_sqrt, cache, coeffs);
            break;
        }
        case kTiltShelf: {
            updateShelfGainLinear<Coeff, kTiltShelf>(n, 0, g_linear_sqrt, cache, coeffs);
            break;
        }
        case kFlatTilt: {
            updateFlatShelfGainLinear<Coeff>(g_linear_sqrt, cache, coeffs);
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
