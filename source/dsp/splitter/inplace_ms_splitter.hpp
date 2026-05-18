// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../vector/vector.hpp"

namespace zldsp::splitter {
    namespace hn = hwy::HWY_NAMESPACE;
    /**
     * a splitter that splits the left/right signal into mid/side signal
     * @tparam FloatType
     */
    template <typename FloatType>
    class InplaceMSSplitter {
    public:
        enum class GainMode {
            kPre, kAvg, kPost
        };

        InplaceMSSplitter() = default;

        /**
         * inplace mid/side split
         * @tparam Mode
         * @param l_buffer input l & output mid
         * @param r_buffer input r & output side
         * @param num_samples
         */
        template <GainMode Mode = GainMode::kPre>
        static void split(FloatType* __restrict l_buffer, FloatType* __restrict r_buffer, const size_t num_samples) {
            static constexpr hn::ScalableTag<FloatType> d;
            static constexpr size_t lanes = hn::MaxLanes(d);
            size_t i = 0;
            if constexpr (Mode == GainMode::kPre) {
                const auto v_half = hn::Set(d, FloatType(0.5));
                for (; i + lanes <= num_samples; i += lanes) {
                    auto v_l = hn::LoadU(d, l_buffer + i);
                    auto v_r = hn::LoadU(d, r_buffer + i);
                    auto v_mid = hn::Mul(v_half, hn::Add(v_l, v_r));
                    auto v_side = hn::Sub(v_mid, v_r);
                    hn::StoreU(v_mid, d, l_buffer + i);
                    hn::StoreU(v_side, d, r_buffer + i);
                }
            }
            else if constexpr (Mode == GainMode::kAvg) {
                const auto v_sqrt2_over_2 = hn::Set(d, kSqrt2Over2);
                const auto v_sqrt2 = hn::Set(d, kSqrt2);
                for (; i + lanes <= num_samples; i += lanes) {
                    auto v_l = hn::LoadU(d, l_buffer + i);
                    auto v_r = hn::LoadU(d, r_buffer + i);
                    auto v_mid = hn::Mul(v_sqrt2_over_2, hn::Add(v_l, v_r));
                    auto v_side = hn::NegMulAdd(v_sqrt2, v_r, v_mid);
                    hn::StoreU(v_mid, d, l_buffer + i);
                    hn::StoreU(v_side, d, r_buffer + i);
                }
            }
            else if constexpr (Mode == GainMode::kPost) {
                const auto v_two = hn::Set(d, FloatType(2));
                for (; i + lanes <= num_samples; i += lanes) {
                    auto v_l = hn::LoadU(d, l_buffer + i);
                    auto v_r = hn::LoadU(d, r_buffer + i);
                    auto v_mid = hn::Add(v_l, v_r);
                    auto v_side = hn::NegMulAdd(v_two, v_r, v_mid);
                    hn::StoreU(v_mid, d, l_buffer + i);
                    hn::StoreU(v_side, d, r_buffer + i);
                }
            }
            for (; i < num_samples; ++i) {
                const auto l = l_buffer[i];
                const auto r = r_buffer[i];
                if constexpr (Mode == GainMode::kPre) {
                    l_buffer[i] = FloatType(0.5) * (l + r);
                    r_buffer[i] = l_buffer[i] - r;
                }
                else if constexpr (Mode == GainMode::kAvg) {
                    l_buffer[i] = kSqrt2Over2 * (l + r);
                    r_buffer[i] = l_buffer[i] - kSqrt2 * r;
                }
                else if constexpr (Mode == GainMode::kPost) {
                    l_buffer[i] = l + r;
                    r_buffer[i] = l_buffer[i] - FloatType(2) * r;
                }
            }
        }

        /**
         * inplace mid/side combine
         * @tparam Mode
         * @param l_buffer input mid & output l
         * @param r_buffer input side & output r
         * @param num_samples
         */
        template <GainMode Mode = GainMode::kPre>
        static void combine(FloatType* __restrict l_buffer, FloatType* __restrict r_buffer, const size_t num_samples) {
            static constexpr hn::ScalableTag<FloatType> d;
            static constexpr size_t lanes = hn::MaxLanes(d);
            size_t i = 0;
            if constexpr (Mode == GainMode::kPre) {
                for (; i + lanes <= num_samples; i += lanes) {
                    auto v_mid = hn::LoadU(d, l_buffer + i);
                    auto v_side = hn::LoadU(d, r_buffer + i);
                    auto v_l = hn::Add(v_mid, v_side);
                    auto v_r = hn::Sub(v_mid, v_side);
                    hn::StoreU(v_l, d, l_buffer + i);
                    hn::StoreU(v_r, d, r_buffer + i);
                }
            }
            else if constexpr (Mode == GainMode::kAvg) {
                const auto v_sqrt2_over_2 = hn::Set(d, kSqrt2Over2);
                const auto v_sqrt2 = hn::Set(d, kSqrt2);
                for (; i + lanes <= num_samples; i += lanes) {
                    auto v_mid = hn::LoadU(d, l_buffer + i);
                    auto v_side = hn::LoadU(d, r_buffer + i);
                    auto v_l = hn::Mul(v_sqrt2_over_2, hn::Add(v_mid, v_side));
                    auto v_r = hn::NegMulAdd(v_sqrt2, v_side, v_l);
                    hn::StoreU(v_l, d, l_buffer + i);
                    hn::StoreU(v_r, d, r_buffer + i);
                }
            }
            else if constexpr (Mode == GainMode::kPost) {
                const auto v_half = hn::Set(d, FloatType(0.5));
                for (; i + lanes <= num_samples; i += lanes) {
                    auto v_mid = hn::LoadU(d, l_buffer + i);
                    auto v_side = hn::LoadU(d, r_buffer + i);
                    auto v_l = hn::Mul(v_half, hn::Add(v_mid, v_side));
                    auto v_r = hn::Sub(v_l, v_side);
                    hn::StoreU(v_l, d, l_buffer + i);
                    hn::StoreU(v_r, d, r_buffer + i);
                }
            }
            for (; i < num_samples; ++i) {
                const auto mid = l_buffer[i];
                const auto side = r_buffer[i];
                if constexpr (Mode == GainMode::kPre) {
                    l_buffer[i] = mid + side;
                    r_buffer[i] = mid - side;
                }
                else if constexpr (Mode == GainMode::kAvg) {
                    l_buffer[i] = kSqrt2Over2 * (mid + side);
                    r_buffer[i] = l_buffer[i] - kSqrt2 * side;
                }
                else if constexpr (Mode == GainMode::kPost) {
                    l_buffer[i] = FloatType(0.5) * (mid + side);
                    r_buffer[i] = l_buffer[i] - side;
                }
            }
        }

    private:
        static constexpr FloatType kSqrt2Over2 = static_cast<FloatType>(
            0.7071067811865475244008443621048490392848359376884740365883398690);
        static constexpr FloatType kSqrt2 = static_cast<FloatType>(
            1.414213562373095048801688724209698078569671875376948073176679738);
    };
}
