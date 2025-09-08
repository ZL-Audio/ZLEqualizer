// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../vector/kfr_import.hpp"

namespace zldsp::splitter {
    /**
     * a splitter that splits the stereo audio signal input mid signal and side signal
     * @tparam FloatType
     */
    template<typename FloatType>
    class InplaceMSSplitter {
    public:
        enum GainMode {
            kPre, kAvg, kPost
        };

        InplaceMSSplitter() = default;

        /**
         * switch left/right buffer to mid/side buffer
         */
        template<GainMode Mode = GainMode::kPre>
        static constexpr void split(FloatType *l_buffer, FloatType *r_buffer, const size_t num_samples) {
            auto l_vector = kfr::make_univector(l_buffer, num_samples);
            auto r_vector = kfr::make_univector(r_buffer, num_samples);

            if constexpr (Mode == GainMode::kPre) {
                l_vector = FloatType(0.5) * (l_vector + r_vector);
                r_vector = l_vector - r_vector;
            } else if constexpr (Mode == GainMode::kAvg) {
                l_vector = kSqrt2Over2 * (l_vector + r_vector);
                r_vector = l_vector - kSqrt2 * r_vector;
            } else if constexpr (Mode == GainMode::kPost) {
                l_vector = l_vector + r_vector;
                r_vector = l_vector - FloatType(2) * r_vector;
            }
        }

        /**
         * switch mis/side buffer to left/right buffer
         */
        template<GainMode Mode = GainMode::kPre>
        static constexpr void combine(FloatType *l_buffer, FloatType *r_buffer, const size_t num_samples) {
            auto l_vector = kfr::make_univector(l_buffer, num_samples);
            auto r_vector = kfr::make_univector(r_buffer, num_samples);

            if constexpr (Mode == GainMode::kPre) {
                l_vector = l_vector + r_vector;
                r_vector = l_vector - FloatType(2) * r_vector;
            } else if constexpr (Mode == GainMode::kAvg) {
                l_vector = kSqrt2Over2 * (l_vector + r_vector);
                r_vector = l_vector - kSqrt2 * r_vector;
            } else if constexpr (Mode == GainMode::kPost) {
                l_vector = FloatType(0.5) * (l_vector + r_vector);
                r_vector = l_vector - r_vector;
            }
        }

    private:
        static constexpr FloatType kSqrt2Over2 = static_cast<FloatType>(
            0.7071067811865475244008443621048490392848359376884740365883398690);
        static constexpr FloatType kSqrt2 = static_cast<FloatType>(
            1.414213562373095048801688724209698078569671875376948073176679738);
    };
}
