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
    HWY_INLINE F dot_product(const F* HWY_RESTRICT in0, const F* HWY_RESTRICT in1,
                             const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        static constexpr size_t block = lanes << 2;

        size_t i = 0;
        hn::Vec<decltype(d)> single_sum;
        if (size >= block) {
            auto sum0 = hn::Zero(d);
            auto sum1 = hn::Zero(d);
            auto sum2 = hn::Zero(d);
            auto sum3 = hn::Zero(d);
            for (; i + block <= size; i += block) {
                {
                    auto va = hn::LoadU(d, in0 + i);
                    auto vb = hn::LoadU(d, in1 + i);
                    sum0 = hn::MulAdd(va, vb, sum0);
                }
                {
                    auto va = hn::LoadU(d, in0 + i + lanes);
                    auto vb = hn::LoadU(d, in1 + i + lanes);
                    sum1 = hn::MulAdd(va, vb, sum1);
                }
                {
                    auto va = hn::LoadU(d, in0 + i + lanes * 2);
                    auto vb = hn::LoadU(d, in1 + i + lanes * 2);
                    sum2 = hn::MulAdd(va, vb, sum2);
                }
                {
                    auto va = hn::LoadU(d, in0 + i + lanes * 3);
                    auto vb = hn::LoadU(d, in1 + i + lanes * 3);
                    sum3 = hn::MulAdd(va, vb, sum3);
                }
            }
            single_sum = hn::Add(hn::Add(sum0, sum1), hn::Add(sum2, sum3));
        } else {
            single_sum = hn::Zero(d);
        }
        for (; i + lanes <= size; i += lanes) {
            auto va = hn::LoadU(d, in0 + i);
            auto vb = hn::LoadU(d, in1 + i);
            single_sum = hn::MulAdd(va, vb, single_sum);
        }
        F scalar_sum = hn::ReduceSum(d, single_sum);
        for (; i < size; ++i) {
            scalar_sum += in0[i] * in1[i];
        }
        return scalar_sum;
    }
}
