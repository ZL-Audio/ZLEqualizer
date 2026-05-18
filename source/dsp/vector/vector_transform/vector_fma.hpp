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
    HWY_INLINE void fma(F* HWY_RESTRICT in, const F to_mul, const F to_add, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        const auto v_mul = hn::Set(d, to_mul);
        const auto v_add = hn::Set(d, to_add);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            const auto v_in = hn::LoadU(d, in + i);
            hn::StoreU(hn::MulAdd(v_mul, v_in, v_add), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] = std::fma(to_mul, in[i], to_add);
        }
    }

    template <typename F>
    HWY_INLINE void fma(F* HWY_RESTRICT out, F* HWY_RESTRICT in, const F to_mul, const F to_add, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        const auto v_mul = hn::Set(d, to_mul);
        const auto v_add = hn::Set(d, to_add);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            const auto v_in = hn::LoadU(d, in + i);
            hn::StoreU(hn::MulAdd(v_mul, v_in, v_add), d, out + i);
        }
        for (; i < size; ++i) {
            out[i] = std::fma(to_mul, in[i], to_add);
        }
    }
}