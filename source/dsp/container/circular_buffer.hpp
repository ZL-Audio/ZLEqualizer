// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>
#include <bit>
#include <algorithm>

namespace zldsp::container {
    /**
     * a circular buffer
     * @tparam T the type of elements
     */
    template <typename T>
    class CircularBuffer {
    public:
        explicit CircularBuffer(const size_t capacity) {
            setCapacity(capacity);
        }

        [[nodiscard]] size_t size() const noexcept {
            return tail_ - head_;
        }

        [[nodiscard]] size_t capacity() const noexcept {
            return mask_;
        }

        [[nodiscard]] bool isEmpty() const noexcept {
            return head_ == tail_;
        }

        void setCapacity(const size_t capacity) {
            size_t real_size = std::bit_ceil(capacity + 1);
            data_.resize(real_size);
            mask_ = real_size - 1;
            clear();
        }

        void clear() noexcept {
            head_ = 0;
            tail_ = 0;
        }

        template <bool boundary_check = true>
        void pushBack(T x) {
            data_[tail_++ & mask_] = x;
            if constexpr (boundary_check) {
                if (size() > mask_) {
                    head_++;
                }
            }
        }

        T popBack() {
            return data_[--tail_ & mask_];
        }

        T& getBack() {
            return data_[(tail_ - 1) & mask_];
        }

        template <bool boundary_check = true>
        void pushFront(T x) {
            data_[--head_ & mask_] = x;
            if constexpr (boundary_check) {
                if (size() > mask_) {
                    tail_--;
                }
            }
        }

        T popFront() {
            return data_[head_++ & mask_];
        }

        T getFront() {
            return data_[head_ & mask_];
        }

        T operator[](const size_t index) {
            return data_[(head_ + index) & mask_];
        }

    private:
        std::vector<T> data_;

        size_t head_{0};
        size_t tail_{0};
        size_t mask_{0};
    };
}
