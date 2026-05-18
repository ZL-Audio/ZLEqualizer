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
    HWY_INLINE void multiply(F* HWY_RESTRICT in, const F to_mul, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        const auto v_m = hn::Set(d, to_mul);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            hn::StoreU(hn::Mul(v_in, v_m), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] *= to_mul;
        }
    }

    template <typename F>
    HWY_INLINE void multiply(F* HWY_RESTRICT out, const F* HWY_RESTRICT in, const F to_mul, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        const auto v_m = hn::Set(d, to_mul);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            hn::StoreU(hn::Mul(v_in, v_m), d, out + i);
        }
        for (; i < size; ++i) {
            out[i] = in[i] * to_mul;
        }
    }

    template <typename F>
    HWY_INLINE void multiply(F* HWY_RESTRICT in, const F* HWY_RESTRICT to_mul, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_mul = hn::LoadU(d, to_mul + i);
            hn::StoreU(hn::Mul(v_in, v_mul), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] *= to_mul[i];
        }
    }

    template <typename F>
    HWY_INLINE void multiply(F* HWY_RESTRICT out, const F* HWY_RESTRICT in, const F* HWY_RESTRICT to_mul,
                             const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_mul = hn::LoadU(d, to_mul + i);
            hn::StoreU(hn::Mul(v_in, v_mul), d, out + i);
        }
        for (; i < size; ++i) {
            out[i] = in[i] * to_mul[i];
        }
    }
}
