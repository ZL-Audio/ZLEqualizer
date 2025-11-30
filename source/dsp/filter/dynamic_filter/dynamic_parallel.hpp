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
    template <typename FloatType, size_t kFilterSize>
    class DynamicParallel final : public DynamicBase<Parallel<FloatType, kFilterSize>, FloatType> {
    public:
        explicit DynamicParallel(DynamicSideHandler<FloatType>& handler) :
            DynamicBase<Parallel<FloatType, kFilterSize>, FloatType>(handler) {
        }

        template <bool bypass = false>
        void processPre(std::span<FloatType*> main_buffer, const size_t num_samples) {
            if constexpr (!bypass) {
                if (this->filter_.getShouldBeParallel()) {
                    const auto parallel_buffer = this->filter_.getParallelBuffer();
                    for (size_t chan = 0; chan < main_buffer.size(); chan++) {
                        zldsp::vector::copy(parallel_buffer[chan], main_buffer[chan], num_samples);
                    }
                }
            }
        }

        /**
         * process the incoming audio buffer
         * @param main_buffer
         * @param side_buffer
         * @param num_samples
         */
        template <bool bypass = false, bool dynamic_on = false, bool dynamic_bypass = false>
        void processDynamic(std::span<FloatType*> main_buffer, std::span<FloatType*> side_buffer,
                            const size_t num_samples) {
            if (this->filter_.getShouldBeParallel()) {
                if constexpr (dynamic_on) {
                    if constexpr (dynamic_bypass) {
                        this->filter_.template setGain<true>(this->handler_.getBaseGain());
                        this->filter_.updateGain();
                    }
                    // make sure that freq & q are update to date
                    if (this->filter_.isFreqQSmoothing()) {
                        this->filter_.skipSmooth();
                        this->filter_.updateCoeffs();
                    }
                    // calculate portion using SIMD
                    this->handler_.process(side_buffer, num_samples);
                    switch (this->handler_.getFollower().getSState()) {
                    case zldsp::compressor::SState::kOff: {
                        internalDynamicProcess<bypass, dynamic_bypass, zldsp::compressor::SState::kOff>(
                            side_buffer, num_samples);
                        break;
                    }
                    case zldsp::compressor::SState::kFull: {
                        internalDynamicProcess<bypass, dynamic_bypass, zldsp::compressor::SState::kFull>(
                            side_buffer, num_samples);
                        break;
                    }
                    case zldsp::compressor::SState::kMix: {
                        internalDynamicProcess<bypass, dynamic_bypass, zldsp::compressor::SState::kMix>(
                            side_buffer, num_samples);
                        break;
                    }
                    default: {
                        break;
                    }
                    }
                } else {
                    const auto parallel_buffer = this->filter_.getParallelBuffer();
                    this->filter_.template process<bypass>(parallel_buffer, num_samples);
                }
            } else {
                DynamicBase<Parallel<FloatType, kFilterSize>, FloatType>::template process<
                    bypass, dynamic_on, dynamic_bypass>(main_buffer, side_buffer, num_samples);
            }
        }

        /**
         * add the parallel buffer to the incoming audio buffer
         * @param main_buffer
         * @param num_samples
         */
        template <bool bypass = false>
        void processPost(std::span<FloatType*> main_buffer, const size_t num_samples) {
            this->filter_.template processPost<bypass>(main_buffer, num_samples);
        }

        [[nodiscard]] bool getShouldBeParallel() const {
            return this->filter_.getShouldBeParallel();
        }

    private:
        template <bool bypass = false, bool dynamic_bypass = false,
                  zldsp::compressor::SState s_state>
        void internalDynamicProcess(std::span<FloatType*> side_buffer, const size_t num_samples) {
            const auto parallel_buffer = this->filter_.getParallelBuffer();
            const auto side_p = side_buffer[0];
            // dynamic processing
            for (size_t i = 0; i < num_samples; ++i) {
                if constexpr (dynamic_bypass) {
                    this->handler_.template getNextGain<s_state>(side_p[i]);
                } else {
                    this->filter_.template setGain<true>(this->handler_.template getNextGain<s_state>(side_p[i]));
                    this->filter_.updateGain();
                }
                const auto multiplier = this->filter_.getMultiplier();
                for (size_t chan = 0; chan < parallel_buffer.size(); ++chan) {
                    if constexpr (bypass) {
                        this->filter_.processSample(chan, parallel_buffer[chan][i]);
                    } else {
                        parallel_buffer[chan][i] = this->filter_.processSample(
                            chan, parallel_buffer[chan][i]) * multiplier;
                    }
                }
            }
        }
    };
}
