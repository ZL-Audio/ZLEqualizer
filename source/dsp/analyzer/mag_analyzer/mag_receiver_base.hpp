// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include "../../vector/vector.hpp"
#include "../../container/fifo/fifo_base.hpp"

namespace zldsp::analyzer {
    struct MagAnalyzerOps {
        /**
         * calculate mean square value of a given buffer over a specified range
         * @param range the specified range
         * @param fifo the buffer
         * @return
         */
        static float calculateMS(const zldsp::container::FIFORange& range,
                                 const std::vector<float>& fifo) {
            if (range.block_size1 + range.block_size2 == 0) {
                return 0.f;
            }

            auto sum_op = [&](const int start, const int size) {
                return vector::sum_sqr(fifo.data() + static_cast<size_t>(start), static_cast<size_t>(size));
            };

            double sum_sqr{0.};
            if (range.block_size1 > 0) {
                sum_sqr += sum_op(range.start_index1, range.block_size1);
            }
            if (range.block_size2 > 0) {
                sum_sqr += sum_op(range.start_index2, range.block_size2);
            }

            const auto total_samples = static_cast<double>(range.block_size1 + range.block_size2);
            return static_cast<float>(sum_sqr / total_samples);
        }

        /**
         * calculate the maximum absolute value of a given buffer over a specified range
         * @param range the specified range
         * @param fifo the buffer
         * @return
         */
        static float calculatePeak(const zldsp::container::FIFORange& range,
                                   const std::vector<float>& fifo) {
            auto max_op = [&](const int start, const int size) {
                return vector::max_abs_of(fifo.data() + static_cast<size_t>(start), static_cast<size_t>(size));
            };

            float peak{0.f};

            if (range.block_size1 > 0) {
                peak = std::max(peak, max_op(range.start_index1, range.block_size1));
            }
            if (range.block_size2 > 0) {
                peak = std::max(peak, max_op(range.start_index2, range.block_size2));
            }
            return peak;
        }
    };
}
