// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "dynamic_side_handler.hpp"

namespace zldsp::filter {
    /**
     * a dynamic IIR filter which processes audio on the real-time thread
     * @tparam FloatType the float type of input audio buffer
     */
    template <class FilterType, typename FloatType>
    class DynamicBase {
    public:
        explicit DynamicBase(DynamicSideHandler<FloatType>& handler) :
            handler_(handler) {
        }

        /**
         * reset IIR filter state and follower state
         */
        void reset() {
            filter_.reset();
        }

        /**
         * reset dynamic status
         */
        void resetDynamic() {
            filter_.setGain(handler_.getBaseGain());
        }

        /**
         * prepare the dynamic filter
         * @param sample_rate
         * @param num_channels
         * @param max_num_samples
         */
        void prepare(const double sample_rate, const size_t num_channels, const size_t max_num_samples) {
            filter_.prepare(sample_rate, num_channels, max_num_samples);
        }

        /**
         *
         * @return the underlying IIR filter
         */
        FilterType& getFilter() {
            return filter_;
        }

    protected:
        FilterType filter_{};
        zldsp::filter::DynamicSideHandler<FloatType>& handler_;

        /**
         * process the incoming audio buffer
         * @param main_buffer
         * @param side_buffer
         * @param num_samples
         */
        template <bool bypass = false, bool dynamic_on = false, bool dynamic_bypass = false>
        void process(std::span<FloatType*> main_buffer, std::span<FloatType*> side_buffer,
                     const size_t num_samples) {
            if constexpr (dynamic_on) {
                using SState = zldsp::compressor::SState;
                switch (handler_.getFollower().getSState()) {
                case SState::kOff: {
                    internalProcess<bypass, dynamic_bypass, SState::kOff>(main_buffer, side_buffer, num_samples);
                    break;
                }
                case SState::kFull: {
                    internalProcess<bypass, dynamic_bypass, SState::kFull>(main_buffer, side_buffer, num_samples);
                    break;
                }
                case SState::kMix: {
                    internalProcess<bypass, dynamic_bypass, SState::kMix>(main_buffer, side_buffer, num_samples);
                    break;
                }
                default: {
                    break;
                }
                }
            } else {
                filter_.template process<bypass>(main_buffer, num_samples);
            }
        }

        template <bool bypass = false, bool dynamic_bypass = false,
                  zldsp::compressor::SState s_state>
        void internalProcess(std::span<FloatType*> main_buffer, std::span<FloatType*> side_buffer,
                             const size_t num_samples) {
            if constexpr (dynamic_bypass) {
                filter_.template setGain<true>(handler_.getBaseGain());
                filter_.updateGain();
            }
            // make sure that freq & q are update to date
            if (filter_.isFreqQSmoothing()) {
                filter_.skipSmooth();
            }
            // calculate portion using SIMD
            handler_.process(side_buffer, num_samples);
            const auto side_p = side_buffer[0];
            // dynamic processing
            for (size_t i = 0; i < num_samples; ++i) {
                if constexpr (dynamic_bypass) {
                    handler_.template getNextGain<s_state>(side_p[i]);
                } else {
                    filter_.template setGain<true>(handler_.template getNextGain<s_state>(side_p[i]));
                    filter_.updateGain();
                }
                for (size_t chan = 0; chan < main_buffer.size(); ++chan) {
                    if constexpr (bypass) {
                        filter_.processSample(chan, main_buffer[chan][i]);
                    } else {
                        main_buffer[chan][i] = filter_.processSample(chan, main_buffer[chan][i]);
                    }
                }
            }
        }
    };
}
