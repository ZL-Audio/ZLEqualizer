// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <type_traits>

#include "ivantsov_coeff.hpp"
#include "ivantsov_svf_coeff.hpp"
#include "ivantsov_common.hpp"
#include "../../helpers.hpp"
#include "../../../vector/highway_import.hpp"

namespace zldsp::filter {
    namespace hn = hwy::HWY_NAMESPACE;

    /**
     * design Ivantsov coefficients for consecutive samples in SIMD lanes
     * @tparam Coeff the coefficient type (IvantsovCoeff or IvantsovSVFCoeff)
     */
    template <class Coeff>
    class IvantsovSIMDCoeff {
        static_assert(std::is_same_v<Coeff, IvantsovCoeff> || std::is_same_v<Coeff, IvantsovSVFCoeff>);

    public:
        static constexpr hn::ScalableTag<double> kTag{};
        static constexpr size_t kLanes = hn::MaxLanes(kTag);
        using Vec = hn::VFromD<decltype(kTag)>;

        struct GainTerms {
            Vec alpha, beta, multiplier;
        };

        template <FilterType type>
        static GainTerms getGainTerms(const Vec gain_sqrt) {
            Vec alpha_gain, beta_gain, multiplier;
            if constexpr (type == kPeak) {
                alpha_gain = gain_sqrt * gain_sqrt;
                beta_gain = broadcast(1.0) / alpha_gain;
                multiplier = broadcast(1.0);
            } else if constexpr (type == kLowShelf) {
                alpha_gain = broadcast(1.0) / gain_sqrt;
                beta_gain = gain_sqrt;
                multiplier = gain_sqrt * gain_sqrt;
            } else {
                alpha_gain = gain_sqrt;
                beta_gain = broadcast(1.0) / gain_sqrt;
                if constexpr (type == kTiltShelf) {
                    multiplier = beta_gain;
                } else {
                    multiplier = broadcast(1.0);
                }
            }
            return {alpha_gain, beta_gain, multiplier};
        }

        template <FilterType type>
        static std::array<Vec, 5> getCoefficients(const GainTerms& gain, const double* cache) {
            constexpr auto kPi = ivantsov::kPi;
            constexpr auto kPiSq = kPi * kPi;
            const auto alpha = getCore<type == kPeak>(gain.alpha, cache);
            const auto beta = getCore<type == kPeak>(gain.beta, cache);
            const auto multiplier = gain.multiplier;
            if constexpr (std::is_same_v<Coeff, IvantsovSVFCoeff>) {
                const auto beta_sqrt = hn::Sqrt(beta.v);
                return {multiplier * alpha.d * broadcast(0.5 / kPiSq), broadcast(2.0 * kPiSq) / beta.d,
                        beta_sqrt * broadcast(1.0 / kPi), multiplier * alpha.v / (broadcast(kPi) * beta_sqrt),
                        multiplier};
            } else {
                const auto inv_d = broadcast(1.0) / beta.d;
                const auto scale = multiplier * inv_d;
                return {(broadcast(2.0 * kPiSq) - broadcast(2.0) * beta.v) * inv_d,
                        (broadcast(kPiSq) - beta.term + beta.v) * inv_d, alpha.d * scale,
                        (broadcast(2.0 * kPiSq) - broadcast(2.0) * alpha.v) * scale,
                        (broadcast(kPiSq) - alpha.term + alpha.v) * scale};
            }
        }

    private:
        struct Core {
            Vec v, d, term;
        };

        static auto broadcast(const double value) { return hn::Set(kTag, value); }

        template <bool peak>
        static Core getCore(const Vec gain, const double* cache) {
            const auto kappa = broadcast(cache[0]) * gain + broadcast(peak ? cache[1] : ivantsov::kSigmaSq);
            Vec v;
            if constexpr (peak) {
                v = hn::Sqrt(hn::Abs(broadcast(cache[2]) * gain + broadcast(cache[3])));
            } else {
                v = hn::Sqrt(hn::Abs(broadcast(cache[1]) * gain * gain + broadcast(cache[2]) * gain
                    + broadcast(ivantsov::kSigmaSq * ivantsov::kSigmaSq)));
            }
            const auto term = broadcast(ivantsov::kPi) * hn::Sqrt(hn::Abs(broadcast(2.0) * (v + kappa)));
            return {v, broadcast(ivantsov::kPi * ivantsov::kPi) + term + v, term};
        }
    };
}
