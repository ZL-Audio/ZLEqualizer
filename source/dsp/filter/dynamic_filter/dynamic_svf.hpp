// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../iir_filter/svf/svf.hpp"
#include "dynamic_base.hpp"

namespace zldsp::filter {
    /**
     * a dynamic IIR filter which processes audio on the real-time thread
     * @tparam FloatType the float type of input audio buffer
     * @tparam FilterSize the number of cascading filters
     */
    template<typename FloatType, size_t FilterSize>
    class DynamicSVF final : DynamicBase<SVF<FloatType, FilterSize>, FloatType, FilterSize> {
    public:
        explicit DynamicSVF(IIREmpty &empty)
            : DynamicBase<SVF<FloatType, FilterSize>, FloatType, FilterSize>(empty) {
        }

        /**
         * process the incoming audio buffer
         * @tparam IsBypassed
         * @param main_buffer
         * @param side_buffer
         * @param num_samples
         */
        template<bool IsBypassed = false>
        void processSVF(std::span<FloatType *> main_buffer, std::span<FloatType *> side_buffer,
                        const size_t num_samples) {
            DynamicBase<SVF<FloatType, FilterSize>, FloatType, FilterSize>::template process<IsBypassed>(
                main_buffer, side_buffer, num_samples);
        }
    };
}
