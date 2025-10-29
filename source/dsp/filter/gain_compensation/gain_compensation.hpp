// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <array>
#include "../helpers.hpp"

namespace zldsp::filter {
    namespace sgc_detail {
        inline constexpr double k1 = 0.165602;
        inline constexpr double k2 = 0.338973;
        inline constexpr double k3 = 0.712232;
        inline constexpr double k4 = 0.374335;
        inline constexpr double k5 = 1.494580;
        inline constexpr double k6 = 7.131157;
        inline constexpr double k7 = 0.014366;
        inline constexpr std::array<double, 3> pps{
            0.6797385437634612,
            0.6501623179337382,
            0.1661043031674446,
        };
        inline constexpr std::array<double, 3> pns{
            1.0005839027125558,
            0.2615438074138483,
            0.0876180361048472,
        };

        inline constexpr std::array<double, 3> lps{
            0.5615303279130026,
            1.0955796383939556,
            0.0578375534446572,
        };
        inline constexpr std::array<double, 3> lns{
            1.7666900390139590,
            -0.9879875452397923,
            0.0466874416227134,
        };

        inline constexpr std::array<double, 3> hps{
            -1.6271905034386083,
            2.6722453328537070,
            0.1780141475194901,
        };
        inline constexpr std::array<double, 3> hns{
            -0.0999799556355004,
            1.0888973867418563,
            0.0760070892708112,
        };

        inline double integrateFQ(const double f1, const double f2) {
            const auto w1 = 1.0000057078597646 + (1.3450513160225395e-8) * f1 * f1;
            const auto w2 = 1.0000057078597646 + (1.3450513160225395e-8) * f2 * f2;
            return std::log((w1 + 1) * (1 - w2) / (w2 + 1) / (1 - w1));
        }

        inline double getEstimation(const double fq_effect, const double bw, const double g,
                                    const std::array<double, 3>& x) {
            return (x[0] * fq_effect + x[1] * bw) * g * x[2];
        }

        inline double getLowShelfGainCompensation(const double freq, const double gain) {
            const auto f = std::clamp(freq, 15.0, 5000.0);
            const auto bw = std::log2(f / 10.0);
            const auto fq_effect = integrateFQ(10.0, f);
            if (gain > 0) {
                return -std::max(0.0, getEstimation(fq_effect, bw, gain, lps));
            } else {
                return -std::min(0.0, getEstimation(fq_effect, bw, gain, lns));
            }
        }

        inline double getHighShelfGainCompensation(const double freq, const double gain) {
            const auto f = std::clamp(freq, 200.0, 19999.0);
            const auto bw = std::log2(20000.0 / f);
            const auto fq_effect = integrateFQ(f, 20000.0);
            if (gain > 0) {
                return -std::max(0.0, getEstimation(fq_effect, bw, gain, hps));
            } else {
                return -std::min(0.0, getEstimation(fq_effect, bw, gain, hns));
            }
        }

        inline double getPeakGainCompensation(const double freq, const double gain, const double q) {
            auto bw = std::asinh(0.5 / q) / std::log(2);
            const auto scale = std::pow(2, bw / 2);
            const auto f1 = std::clamp(freq / scale, 10.0, 20000.0);
            const auto f2 = std::clamp(freq * scale, 10.0, 20000.0);
            bw = std::log2(std::max(0.0, f2 / f1)) * 2.0;
            const auto fq_effect = integrateFQ(f1, f2);
            if (gain > 0) {
                return -std::max(0.0, getEstimation(fq_effect, bw, gain, pps));
            } else {
                return -std::min(0.0, getEstimation(fq_effect, bw, gain, pns));
            }
        }
    }

    inline double getGainCompensation(const FilterParameters& paras) {
        switch (paras.filter_type) {
        case kPeak:
        case kBandShelf: {
            return sgc_detail::getPeakGainCompensation(paras.freq, paras.gain, paras.q);
        }
        case kLowShelf: {
            return sgc_detail::getLowShelfGainCompensation(paras.freq, paras.gain);
        }
        case kHighShelf: {
            return sgc_detail::getHighShelfGainCompensation(paras.freq, paras.gain);
        }
        case kLowPass:
        case kHighPass:
        case kBandPass:
        case kNotch:
        case kTiltShelf:
        default:
            return 0.0;
        }
    }
}
