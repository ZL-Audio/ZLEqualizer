// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>

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

        [[nodiscard]] size_t capacity() const { return data_.size() - 1; }

        [[nodiscard]] size_t size() const {
            return tail_ >= head_
                       ? static_cast<size_t>(tail_ - head_)
                       : static_cast<size_t>(tail_ + static_cast<unsigned int>(data_.size()) - head_);
        }

        [[nodiscard]] bool isEmpty() const {
            return tail_ == head_;
        }

        void setCapacity(const size_t capacity) {
            data_.resize(capacity + 1);
            clear();
        }

        void clear() {
            head_ = 0;
            tail_ = 0;
        }

        void pushBack(T x) {
            tail_ = (tail_ + 1) % static_cast<unsigned int>(data_.size());
            data_[static_cast<size_t>(tail_)] = x;
            if (tail_ == head_) {
                popFront();
            }
        }

        T popBack() {
            const auto t = data_[static_cast<size_t>(tail_)];
            tail_ = tail_ > 0 ? tail_ - 1 : static_cast<unsigned int>(data_.size()) - 1;
            return t;
        }

        T getBack() {
            return data_[static_cast<size_t>(tail_)];
        }

        void pushFront(T x) {
            data_[static_cast<size_t>(head_)] = x;
            head_ = head_ > 0 ? head_ - 1 : static_cast<unsigned int>(data_.size()) - 1;
            if (head_ == tail_) {
                popBack();
            }
        }

        T popFront() {
            head_ = (head_ + 1) % static_cast<unsigned int>(data_.size());
            return data_[static_cast<size_t>(head_)];
        }

        T getFront() {
            return data_[static_cast<size_t>((head_ + 1) % static_cast<unsigned int>(data_.size()))];
        }

    private:
        std::vector<T> data_;
        unsigned int head_{0}, tail_{0};
    };
}
