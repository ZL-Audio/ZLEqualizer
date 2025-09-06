// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../iir_filter/parallel/parallel.hpp"
#include "dynamic_base.hpp"

namespace zldsp::filter {
    /**
     * a dynamic IIR filter which processes audio on the real-time thread
     * @tparam FloatType the float type of input audio buffer
     * @tparam kFilterSize the number of cascading filters
     */
    template<typename FloatType, size_t kFilterSize>
    class DynamicParallel final : public DynamicBase<Parallel<FloatType, kFilterSize>, FloatType> {
    public:
        DynamicParallel() : DynamicBase<Parallel<FloatType, kFilterSize>, FloatType>() {
        }

        /**
         * process the incoming audio buffer
         * @tparam bypass
         * @param main_buffer
         * @param side_buffer
         * @param num_samples
         */
        template<bool bypass = false>
        void processParallel(std::span<FloatType *> main_buffer, std::span<FloatType *> side_buffer,
                             const size_t num_samples) {
            if (this->filter_.getShouldBeParallel()) {
                zldsp::vector::copy(this->filter_.getParallelBuffer(), main_buffer, num_samples);
                DynamicBase<Parallel<FloatType, kFilterSize>, FloatType>::template process<bypass>(
                    this->filter_.getParallelBuffer(), side_buffer, num_samples);
            } else {
                DynamicBase<Parallel<FloatType, kFilterSize>, FloatType>::template process<bypass>(
                    main_buffer, side_buffer, num_samples);
            }
        }

        /**
         * add the parallel buffer to the incoming audio buffer
         * @param main_buffer
         * @param num_samples
         */
        template<bool IsBypassed = false>
        void processPost(std::span<FloatType *> main_buffer, const size_t num_samples) {
            this->filter_.template processPost<IsBypassed>(main_buffer, num_samples);
        }
    };
}
