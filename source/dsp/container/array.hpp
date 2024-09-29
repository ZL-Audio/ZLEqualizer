// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLCONTAINER_FIXED_MAX_SIZE_ARRAY_HPP
#define ZLCONTAINER_FIXED_MAX_SIZE_ARRAY_HPP

namespace zlContainer {
    template<typename T, size_t N>
    class FixedMaxSizeArray {
    public:
        std::array<T, N> data{};
        size_t size = 0;

        FixedMaxSizeArray() = default;

        FixedMaxSizeArray &operator =(const FixedMaxSizeArray<T, N> &that) {
            for (size_t i = 0; i < size; ++i) {
                data[i] = that.data[i];
            }
            return *this;
        }

        void push(const T x) {
            if (size == N) {
                size = 0;
            }
            data[size] = x;
            size++;
        }

        void clear() {
            size = 0;
        }
    };
}

#endif //ZLCONTAINER_FIXED_MAX_SIZE_ARRAY_HPP
