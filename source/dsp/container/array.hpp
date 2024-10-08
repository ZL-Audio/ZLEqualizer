// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_CONTAINER_FIXED_MAX_SIZE_ARRAY_HPP
#define ZL_CONTAINER_FIXED_MAX_SIZE_ARRAY_HPP

#include <array>

namespace zlContainer {
    template<typename T, size_t N>
    class FixedMaxSizeArray {
    public:
        FixedMaxSizeArray() = default;

        FixedMaxSizeArray &operator =(const FixedMaxSizeArray<T, N> &that) {
            for (size_t i = 0; i < mSize; ++i) {
                data[i] = that.data[i];
            }
            return *this;
        }

        T operator [](const size_t idx) const {
            return data[idx];
        }

        void push(const T x) {
            if (mSize == N) {
                mSize = 0;
            }
            data[mSize] = x;
            mSize++;
        }

        void clear() {
            mSize = 0;
        }

        [[nodiscard]] size_t size() const {
            return mSize;
        }

        [[nodiscard]] static size_t capacity() {
            return N;
        }

    private:
        std::array<T, N> data{};
        size_t mSize = 0;
    };
}

#endif //ZL_CONTAINER_FIXED_MAX_SIZE_ARRAY_HPP
