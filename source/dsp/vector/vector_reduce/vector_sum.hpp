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
    HWY_INLINE F sum(const F* HWY_RESTRICT in, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        static constexpr size_t block = lanes << 2;

        size_t i = 0;
        auto single_sum = hn::Zero(d);
        if (size >= block) {
            auto sum0 = hn::Zero(d);
            auto sum1 = hn::Zero(d);
            auto sum2 = hn::Zero(d);
            auto sum3 = hn::Zero(d);
            for (; i + block <= size; i += block) {
                sum0 = hn::Add(sum0, hn::LoadU(d, in + i));
                sum1 = hn::Add(sum1, hn::LoadU(d, in + i + lanes));
                sum2 = hn::Add(sum2, hn::LoadU(d, in + i + lanes * 2));
                sum3 = hn::Add(sum3, hn::LoadU(d, in + i + lanes * 3));
            }
            single_sum = hn::Add(hn::Add(sum0, sum1), hn::Add(sum2, sum3));
        }
        for (; i + lanes <= size; i += lanes) {
            auto va = hn::LoadU(d, in + i);
            single_sum = hn::Add(va, single_sum);
        }
        F scalar_sum = hn::ReduceSum(d, single_sum);
        for (; i < size; ++i) {
            scalar_sum += in[i];
        }
        return scalar_sum;
    }
}
