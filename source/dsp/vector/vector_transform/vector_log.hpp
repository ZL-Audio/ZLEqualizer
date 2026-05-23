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

    template <typename F, bool use_min = false>
    HWY_INLINE void log(F* __restrict in, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        static constexpr auto kLogMin = static_cast<F>(1e-12);
        const auto v_min = hn::Set(d, kLogMin);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            const auto v_in = hn::LoadU(d, in + i);
            if constexpr (use_min) {
                const auto v_log = hn::Log(d, hn::Max(v_in, v_min));
                hn::StoreU(v_log, d, in + i);
            } else {
                const auto v_log = hn::Log(d, v_in);
                hn::StoreU(v_log, d, in + i);
            }
        }
        for (; i < size; ++i) {
            if constexpr (use_min) {
                in[i] = std::log(std::max(in[i], kLogMin));
            } else {
                in[i] = std::log(in[i]);
            }
        }
    }
}