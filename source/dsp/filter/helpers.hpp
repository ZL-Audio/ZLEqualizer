// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#ifndef ZL_ENABLE_FILTER_SCALED_PASS
#define ZL_ENABLE_FILTER_SCALED_PASS 1
#endif

#include <cmath>
#include <array>
#include <numeric>
#include <numbers>

#include "../chore/decibels.hpp"

namespace zldsp::filter {
    inline constexpr double pi = std::numbers::pi;
    inline constexpr double ppi = 2 * std::numbers::pi;

    inline constexpr double kDbToExp2 = 0.16609640474436813;
    inline constexpr double kDbToExp2Sqrt = kDbToExp2 * 0.5;

    enum FilterType {
        kPeak, kLowShelf, kLowPass, kHighShelf, kHighPass,
        kNotch, kBandPass, kTiltShelf, kFlatTilt, kAllPass, kFlatGain,
#if ZL_ENABLE_FILTER_SCALED_PASS
        kScaledLowPass, kScaledHighPass
#endif
    };

#if ZL_ENABLE_FILTER_SCALED_PASS
    inline constexpr bool isScaledPass(const FilterType type) {
        return type == kScaledLowPass || type == kScaledHighPass;
    }
#endif

    inline constexpr bool isLowPass(const FilterType type) {
#if ZL_ENABLE_FILTER_SCALED_PASS
        if (type == kScaledLowPass) { return true; }
#endif
        return type == kLowPass;
    }

    inline constexpr bool isHighPass(const FilterType type) {
#if ZL_ENABLE_FILTER_SCALED_PASS
        if (type == kScaledHighPass) { return true; }
#endif
        return type == kHighPass;
    }

    inline constexpr bool isPass(const FilterType type) {
        return isLowPass(type) || isHighPass(type);
    }

    enum FilterStructure {
        kIIR, kSVF, kParallel
    };

    struct FilterParameters {
        FilterType filter_type;
        size_t order;
        double freq, gain, q;
    };

    inline double dotProduct(const std::array<double, 3>& x, const std::array<double, 3>& y) {
        return x[0] * y[0] + x[1] * y[1] + x[2] * y[2];
    }

    inline double gainToDB(const double gain) {
        return chore::gainToDecibels(gain);
    }

    inline double dbToGain(const double db) {
        return chore::decibelsToGain(db);
    }

    inline std::array<double, 2> getBandwidth(const double w0, const double q) {
        const auto bw = 2 * std::asinh(0.5 / q) / std::log(2);
        const auto scale = std::pow(2, bw / 2);
        return {w0 / scale, w0 * scale};
    }

    class PassQDistribution {
    public:
        PassQDistribution(const size_t order, const double q) :
            number_(static_cast<double>(order / 2)), q_(q) {
            if (order > 2) {
                theta_ = pi / number_ / 4.0;
                scale_ = std::pow(std::sqrt(2.0) * q, 1.0 / number_);
                rescale_ = std::log10(std::sqrt(2.0) * q) / std::pow(static_cast<double>(order), 1.5) * 12.0;
            }
        }

        [[nodiscard]] double get(const size_t i) const {
            if (number_ <= 1.0) { return q_; }
            const auto centered = static_cast<double>(i) - number_ / 2.0 + 0.5;
            return 1.0 / 2.0 / std::cos(theta_ * static_cast<double>(2 * i + 1))
                   * scale_ * std::pow(2.0, centered * rescale_);
        }

    private:
        double number_, q_, theta_{}, scale_{}, rescale_{};
    };
}
