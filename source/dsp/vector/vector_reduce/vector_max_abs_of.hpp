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
    HWY_INLINE F max_abs_of(const F* __restrict in, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        auto v_max_abs = hn::Zero(d);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_abs = hn::Abs(v_in);
            v_max_abs = hn::Max(v_max_abs, v_abs);
        }
        F scalar_max_abs = hn::ReduceMax(d, v_max_abs);
        for (; i < size; ++i) {
            scalar_max_abs = std::max(scalar_max_abs, std::abs(in[i]));
        }
        return scalar_max_abs;
    }
}
