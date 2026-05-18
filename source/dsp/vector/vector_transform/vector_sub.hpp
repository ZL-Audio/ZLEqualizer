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
    HWY_INLINE void sub(F* HWY_RESTRICT in, const F* HWY_RESTRICT to_sub, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_add = hn::LoadU(d, to_sub + i);
            hn::StoreU(hn::Sub(v_in, v_add), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] -= to_sub[i];
        }
    }

    template <typename F>
    HWY_INLINE void sub(F* HWY_RESTRICT out, const F* HWY_RESTRICT in, const F* HWY_RESTRICT to_sub,
                        const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_add = hn::LoadU(d, to_sub + i);
            hn::StoreU(hn::Sub(v_in, v_add), d, out + i);
        }
        for (; i < size; ++i) {
            out[i] = in[i] - to_sub[i];
        }
    }
}
