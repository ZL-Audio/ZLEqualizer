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
    HWY_INLINE F sum_sqr(const F* HWY_RESTRICT in, const size_t size) {
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
                    auto va = hn::LoadU(d, in + i);
                    sum0 = hn::MulAdd(va, va, sum0);
                }
                {
                    auto va = hn::LoadU(d, in + i + lanes);
                    sum1 = hn::MulAdd(va, va, sum1);
                }
                {
                    auto va = hn::LoadU(d, in + i + lanes * 2);
                    sum2 = hn::MulAdd(va, va, sum2);
                }
                {
                    auto va = hn::LoadU(d, in + i + lanes * 3);
                    sum3 = hn::MulAdd(va, va, sum3);
                }
            }
            single_sum = hn::Add(hn::Add(sum0, sum1), hn::Add(sum2, sum3));
        } else {
            single_sum = hn::Zero(d);
        }
        for (; i + lanes <= size; i += lanes) {
            auto va = hn::LoadU(d, in + i);
            single_sum = hn::MulAdd(va, va, single_sum);
        }
        F scalar_sum = hn::ReduceSum(d, single_sum);
        for (; i < size; ++i) {
            scalar_sum += in[i] * in[i];
        }
        return scalar_sum;
    }
}
