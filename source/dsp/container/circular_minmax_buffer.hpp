// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "circular_buffer.hpp"

namespace zldsp::container {
    /**
     * a circular buffer that can track the min/max value
     * @tparam T the type of elements
     * @tparam FindMin
     * @tparam FindMax
     */
    enum MinMaxBufferType {
        kFindMin, kFindMax
    };

    template <typename T, MinMaxBufferType BufferType>
    class CircularMinMaxBuffer {
    public:
        explicit CircularMinMaxBuffer(const size_t capacity = 1) {
            setCapacity(capacity);
        }

        void setCapacity(const size_t capacity) {
            minmax_buffer_.setCapacity(capacity);
        }

        void setSize(const size_t x) {
            size_ = static_cast<unsigned long long>(x);
            if (static_cast<unsigned long long>(x) < count_) {
                while (!minmax_buffer_.isEmpty() && minmax_buffer_.getFront().second <= head_ - count_) {
                    minmax_buffer_.popFront();
                }
            }
        }

        [[nodiscard]] size_t getSize() const { return size_; }

        void clear() {
            count_ = 0;
        }

        T push(T x) {
            // increment head and count
            head_++;
            count_ = std::min(count_ + 1, size_);
            // prevent overflow (very unlikely)
            if (head_ == std::numeric_limits<unsigned long long>::max()) {
                const auto shift = minmax_buffer_.getFront().second;
                for (size_t i = 0; i < minmax_buffer_.size(); ++i) {
                    auto front = minmax_buffer_.popFront();
                    front.second -= shift;
                    minmax_buffer_.pushBack(front);
                }
                head_ -= shift;
            }
            // remove samples which fail out of the window
            while (!minmax_buffer_.isEmpty() && minmax_buffer_.getFront().second <= head_ - count_) {
                minmax_buffer_.popFront();
            }
            // maintain monotonicity
            if constexpr (BufferType == kFindMin) {
                while (!minmax_buffer_.isEmpty() && minmax_buffer_.getBack().first >= x) {
                    minmax_buffer_.popBack();
                }
            }
            if constexpr (BufferType == kFindMax) {
                while (!minmax_buffer_.isEmpty() && minmax_buffer_.getBack().first <= x) {
                    minmax_buffer_.popBack();
                }
            }
            // push the new sample
            minmax_buffer_.pushBack({x, head_});

            return minmax_buffer_.getFront().first;
        }

    private:
        unsigned long long head_{0}, count_{0}, size_{0};
        CircularBuffer<std::pair<T, unsigned long long>> minmax_buffer_{1};
    };
}
