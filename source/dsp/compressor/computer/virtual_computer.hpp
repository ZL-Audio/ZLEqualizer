// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

//
// Created by Zishu Liu on 12/24/23.
//

#ifndef ZLEQUALIZER_VIRTUAL_COMPUTER_HPP
#define ZLEQUALIZER_VIRTUAL_COMPUTER_HPP

namespace zlCompressor {
    template<typename FloatType>
    class VirtualComputer {
    public:
        VirtualComputer() = default;

        virtual ~VirtualComputer() = default;

        virtual FloatType process(FloatType x) = 0;
    };
}

#endif //ZLEQUALIZER_VIRTUAL_COMPUTER_HPP
