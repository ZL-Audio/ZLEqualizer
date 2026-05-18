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
    HWY_INLINE void clamp(F* HWY_RESTRICT in, const F lo, const F hi, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        const auto v_lo = hn::Set(d, lo);
        const auto v_hi = hn::Set(d, hi);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            hn::StoreU(hn::Clamp(v_in, v_lo, v_hi), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] = std::clamp(in[i], lo, hi);
        }
    }
}
