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
#include <array>
#include <complex>

namespace zldsp::filter {
    template <typename FloatType>
    class IdealBase {
    public:
        template <bool replace = true>
        static void updateMagnitudeSquare(const std::array<double, 5>& coeff,
                                          std::span<const FloatType> ws,
                                          std::span<FloatType> res_mag_sqr) {

            static constexpr hn::ScalableTag<FloatType> d;
            static constexpr size_t lanes = hn::MaxLanes(d);

            const auto a1_sq = static_cast<FloatType>(coeff[0] * coeff[0]);
            const auto a0 = static_cast<FloatType>(coeff[1]);
            const auto b2 = static_cast<FloatType>(coeff[2]);
            const auto b1_sq = static_cast<FloatType>(coeff[3] * coeff[3]);
            const auto b0 = static_cast<FloatType>(coeff[4]);

            const auto va1_sq = hn::Set(d, a1_sq);
            const auto va0 = hn::Set(d, a0);
            const auto vb2 = hn::Set(d, b2);
            const auto vb1_sq = hn::Set(d, b1_sq);
            const auto vb0 = hn::Set(d, b0);

            size_t i = 0;
            for (; i + lanes <= ws.size(); i += lanes) {
                const auto w = hn::LoadU(d, ws.data() + i);
                const auto w2 = hn::Mul(w, w);

                const auto t1 = hn::Sub(va0, w2);
                const auto den = hn::MulAdd(va1_sq, w2, hn::Mul(t1, t1));

                const auto t2 = hn::Sub(vb0, hn::Mul(vb2, w2));
                const auto num = hn::MulAdd(vb1_sq, w2, hn::Mul(t2, t2));
                const auto mag_sq = hn::Div(num, den);

                if constexpr (replace) {
                    hn::StoreU(mag_sq, d, res_mag_sqr.data() + i);
                } else {
                    const auto g = hn::LoadU(d, res_mag_sqr.data() + i);
                    hn::StoreU(hn::Mul(g, mag_sq), d, res_mag_sqr.data() + i);
                }
            }

            for (; i < ws.size(); ++i) {
                const auto w = ws[i];
                const auto w_2 = w * w;
                const auto t1 = a0 - w_2;
                const auto den = a1_sq * w_2 + t1 * t1;
                const auto t2 = b0 - b2 * w_2;
                const auto num = b1_sq * w_2 + t2 * t2;
                const auto mag_sq = num / den;

                if constexpr (replace) {
                    res_mag_sqr[i] = mag_sq;
                } else {
                    res_mag_sqr[i] *= mag_sq;
                }
            }
        }

        template <bool replace = true>
        static void updateResponse(const std::array<double, 5>& coeff,
                                   std::span<const FloatType> ws,
                                   std::span<FloatType> res_real, std::span<FloatType> res_imag) {

            static constexpr hn::ScalableTag<FloatType> d;
            static constexpr size_t lanes = hn::MaxLanes(d);

            const auto a1 = static_cast<FloatType>(coeff[0]);
            const auto a0 = static_cast<FloatType>(coeff[1]);
            const auto b2 = static_cast<FloatType>(coeff[2]);
            const auto b1 = static_cast<FloatType>(coeff[3]);
            const auto b0 = static_cast<FloatType>(coeff[4]);

            const auto va1 = hn::Set(d, a1);
            const auto va0 = hn::Set(d, a0);
            const auto vb2 = hn::Set(d, b2);
            const auto vb1 = hn::Set(d, b1);
            const auto vb0 = hn::Set(d, b0);

            size_t i = 0;
            for (; i + lanes <= ws.size(); i += lanes) {
                const auto w = hn::LoadU(d, ws.data() + i);
                const auto w2 = hn::Mul(w, w);

                const auto nr = hn::Sub(vb0, hn::Mul(vb2, w2));
                const auto ni = hn::Mul(vb1, w);

                const auto dr = hn::Sub(va0, w2);
                const auto di = hn::Mul(va1, w);

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

            for (; i < ws.size(); ++i) {
                const auto w = ws[i];
                const auto w2 = w * w;

                const auto nr = b0 - b2 * w2;
                const auto ni = b1 * w;

                const auto dr = a0 - w2;
                const auto di = a1 * w;

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

        static void calculateWs(std::span<FloatType> ws) {
            ws.front() = static_cast<FloatType>(std::numbers::pi * 0.00001);
            const auto delta = std::numbers::pi / static_cast<double>(ws.size() - 1);
            for (size_t i = 1; i < ws.size() - 1; ++i) {
                ws[i] = static_cast<FloatType>(static_cast<double>(i) * delta);
            }
            ws.back() = static_cast<FloatType>(std::numbers::pi * 0.99999);
        }
    };
}
