// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

namespace zldsp::compressor {
    template<typename FloatType>
    class ComputerBase {
    public:
        ComputerBase() = default;

        virtual ~ComputerBase() = default;

        /**
         * call before processing starts
         * @param sr sampleRate
         */
        virtual void prepare([[maybe_unused]] double sr) {}

        /**
         * reset the computer
         */
        virtual void reset([[maybe_unused]] FloatType x) {}

        /**
         * update values before processing a buffer
         * @return whether the computer has been updated
         */
        virtual bool prepareBuffer() = 0;

        /**
         * @param x
         * @return computer function value at x
         */
        virtual FloatType eval(FloatType x) = 0;
    };
}
