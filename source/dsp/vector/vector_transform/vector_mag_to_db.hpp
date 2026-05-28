// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../highway_import.hpp"

namespace zldsp::vector {
    namespace hn = hwy::HWY_NAMESPACE;

    template <typename F>
    HWY_INLINE void mag_to_db(F* __restrict in, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        static constexpr auto kLogMin = static_cast<F>(1e-12);
        static constexpr auto kLogMul = static_cast<F>(8.6858896380650365530);
        const auto v_min = hn::Set(d, kLogMin);
        const auto v_multiplier = hn::Set(d, kLogMul);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_log = hn::Log(d, hn::Max(v_in, v_min));
            hn::StoreU(hn::Mul(v_log, v_multiplier), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] = kLogMul * std::log(std::max(in[i], kLogMin));
        }
    }

    template <typename F>
    HWY_INLINE void sqr_mag_to_db(F* __restrict in, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        static constexpr auto kLogSqrMin = static_cast<F>(1e-24);
        static constexpr auto kLogSqrMul = static_cast<F>(4.3429448190325182765);
        const auto v_min = hn::Set(d, kLogSqrMin);
        const auto v_multiplier = hn::Set(d, kLogSqrMul);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_log = hn::Log(d, hn::Max(v_in, v_min));
            hn::StoreU(hn::Mul(v_log, v_multiplier), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] = kLogSqrMul * std::log(std::max(in[i], kLogSqrMin));
        }
    }

    template <typename F>
    HWY_INLINE void sqr_mag_to_db(F* __restrict out, F* __restrict in, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        static constexpr auto kLogSqrMin = static_cast<F>(1e-24);
        static constexpr auto kLogSqrMul = static_cast<F>(4.3429448190325182765);
        const auto v_min = hn::Set(d, kLogSqrMin);
        const auto v_multiplier = hn::Set(d, kLogSqrMul);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_log = hn::Log(d, hn::Max(v_in, v_min));
            hn::StoreU(hn::Mul(v_log, v_multiplier), d, out + i);
        }
        for (; i < size; ++i) {
            out[i] = kLogSqrMul * std::log(std::max(in[i], kLogSqrMin));
        }
    }
}
