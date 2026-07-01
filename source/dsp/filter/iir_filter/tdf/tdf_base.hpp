// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <numbers>
#include <vector>
#include <span>

#include "../../../vector/vector.hpp"

namespace zldsp::filter {
    namespace hn = hwy::HWY_NAMESPACE;

    template <typename FloatType>
    class TDFBase {
    public:
        template <bool replace = true>
        static void updateResponse(const std::array<double, 5>& coeff,
                                   std::span<const FloatType> wi_real, std::span<const FloatType> wi_imag,
                                   std::span<FloatType> res_real, std::span<FloatType> res_imag) {
            static constexpr hn::ScalableTag<FloatType> d;
            static constexpr size_t lanes = hn::MaxLanes(d);

            const auto a1 = static_cast<FloatType>(coeff[0]);
            const auto a2 = static_cast<FloatType>(coeff[1]);
            const auto b0 = static_cast<FloatType>(coeff[2]);
            const auto b1 = static_cast<FloatType>(coeff[3]);
            const auto b2 = static_cast<FloatType>(coeff[4]);

            const auto va1 = hn::Set(d, a1);
            const auto va2 = hn::Set(d, a2);
            const auto vb0 = hn::Set(d, b0);
            const auto vb1 = hn::Set(d, b1);
            const auto vb2 = hn::Set(d, b2);
            const auto v_one = hn::Set(d, static_cast<FloatType>(1.0));
            const auto v_two = hn::Set(d, static_cast<FloatType>(2.0));

            size_t i = 0;
            for (; i + lanes <= wi_real.size(); i += lanes) {
                const auto wr = hn::LoadU(d, wi_real.data() + i);
                const auto wi = hn::LoadU(d, wi_imag.data() + i);
                const auto wr2 = hn::Sub(hn::Mul(wr, wr), hn::Mul(wi, wi));
                const auto wi2 = hn::Mul(v_two, hn::Mul(wr, wi));

                const auto nr = hn::MulAdd(vb2, wr2, hn::MulAdd(vb1, wr, vb0));
                const auto ni = hn::MulAdd(vb2, wi2, hn::Mul(vb1, wi));

                const auto dr = hn::MulAdd(va2, wr2, hn::MulAdd(va1, wr, v_one)); // FIXED
                const auto di = hn::MulAdd(va2, wi2, hn::Mul(va1, wi));

                const auto den = hn::Add(hn::Mul(dr, dr), hn::Mul(di, di));
                const auto hr = hn::Div(hn::Add(hn::Mul(nr, dr), hn::Mul(ni, di)), den);
                const auto hi = hn::Div(hn::Sub(hn::Mul(ni, dr), hn::Mul(nr, di)), den);

                if constexpr (replace) {
                    hn::StoreU(hr, d, res_real.data() + i);
                    hn::StoreU(hi, d, res_imag.data() + i);
                } else {
                    const auto rr = hn::LoadU(d, res_real.data() + i);
                    const auto ri = hn::LoadU(d, res_imag.data() + i);
                    const auto new_rr = hn::Sub(hn::Mul(rr, hr), hn::Mul(ri, hi));
                    const auto new_ri = hn::Add(hn::Mul(rr, hi), hn::Mul(ri, hr));

                    hn::StoreU(new_rr, d, res_real.data() + i);
                    hn::StoreU(new_ri, d, res_imag.data() + i);
                }
            }

            for (; i < wi_real.size(); ++i) {
                const auto wr = wi_real[i];
                const auto wi = wi_imag[i];
                const auto wr2 = wr * wr - wi * wi;
                const auto wi2 = static_cast<FloatType>(2.0) * wr * wi;

                const auto nr = b0 + b1 * wr + b2 * wr2;
                const auto ni = b1 * wi + b2 * wi2;
                const auto dr = static_cast<FloatType>(1.0) + a1 * wr + a2 * wr2;
                const auto di = a1 * wi + a2 * wi2;

                const auto den = dr * dr + di * di;
                const auto hr = (nr * dr + ni * di) / den;
                const auto hi = (ni * dr - nr * di) / den;

                if constexpr (replace) {
                    res_real[i] = hr;
                    res_imag[i] = hi;
                } else {
                    const auto rr = res_real[i];
                    const auto ri = res_imag[i];
                    res_real[i] = rr * hr - ri * hi;
                    res_imag[i] = rr * hi + ri * hr;
                }
            }
        }

        static void calculateWs(std::span<FloatType> wi_real, std::span<FloatType> wi_imag) {
            {
                constexpr auto w = std::numbers::pi * 0.00001;
                wi_real.front() = static_cast<FloatType>(std::cos(w));
                wi_imag.front() = static_cast<FloatType>(-std::sin(w));
            }
            const auto delta = std::numbers::pi / static_cast<double>(wi_real.size() - 1);
            for (size_t i = 1; i < wi_real.size() - 1; ++i) {
                const auto w = static_cast<double>(i) * delta;
                wi_real[i] = static_cast<FloatType>(std::cos(w));
                wi_imag[i] = static_cast<FloatType>(-std::sin(w));
            }
            {
                constexpr auto w = std::numbers::pi * 0.99999;
                wi_real.back() = static_cast<FloatType>(std::cos(w));
                wi_imag.back() = static_cast<FloatType>(-std::sin(w));
            }
        }
    };
}
