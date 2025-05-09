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
    template<typename T>
    class CircularBuffer {
    public:
        explicit CircularBuffer(const size_t capacity) {
            data_.resize(capacity);
        }

        [[nodiscard]] size_t capacity() const { return data_.size(); }

        [[nodiscard]] size_t size() const { return static_cast<size_t>(c_num_); }

        void setCapacity(const size_t capacity) {
            data_.resize(capacity);
        }

        void clear() {
            std::fill(data_.begin(), data_.end(), T());
            pos_ = 0;
            c_num_ = 0;
        }

        void pushBack(T x) {
            data_[static_cast<size_t>(pos_)] = x;
            pos_ = (pos_ + 1) % static_cast<int>(data_.size());
            c_num_ = std::min(c_num_ + 1, static_cast<int>(data_.size()));
        }

        T popFront() {
            const auto front_pos = (pos_ - c_num_ + static_cast<int>(data_.size())) % static_cast<int>(data_.size());
            c_num_ -= 1;
            return data_[static_cast<size_t>(front_pos)];
        }

    private:
        std::vector<T> data_;
        int pos_ = 0, c_num_ = 0;
    };
}
