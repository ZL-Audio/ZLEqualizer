// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>

#include "../iir_filter/iir/iir_empty.hpp"
#include "../../compressor/follower/ps_follower.hpp"
#include "../../chore/decibels.hpp"

namespace zldsp::filter {
    /**
     * a dynamic IIR filter which processes audio on the real-time thread
     * @tparam FloatType the float type of input audio buffer
     * @tparam FilterSize the number of cascading filters
     */
    template<class FilterType, typename FloatType, size_t FilterSize>
    class DynamicBase {
    public:
        explicit DynamicBase(IIREmpty &empty) : empty_(empty), filter_(empty) {
        }

        void reset() {
            filter_.reset();
            follower_.reset();
        }

        void forceUpdate() {
            filter_.forceUpdate();
        }

        void prepareBuffer(const FloatType threshold_shift) {
            if (dynamic_on_.load(std::memory_order::relaxed) != c_dynamic_on_) {
                c_dynamic_on_ = dynamic_on_.load(std::memory_order::relaxed);
                // reset gain to base gain
                filter_.setGain(empty_.getGain());
                follower_.reset();
            }
            if (c_dynamic_on_) {
                follower_.prepareBuffer();
                c_base_gain_ = empty_.getGain();
                c_gain_diff_ = target_gain_.load(std::memory_order::relaxed) - c_base_gain_;
                updateThreshold(threshold_shift);
            }
            filter_.prepareBuffer();
        }

        void prepare(const double sample_rate, const size_t num_channels, const size_t max_num_samples) {
            filter_.prepare(sample_rate, num_channels, max_num_samples);
            follower_.prepare(sample_rate);
        }

        /**
         * process the incoming audio buffer
         * @tparam IsBypassed
         * @param main_buffer
         * @param side_buffer
         * @param num_samples
         */
        template<bool IsBypassed = false>
        void process(std::span<FloatType *> main_buffer, std::span<FloatType *> side_buffer,
                     const size_t num_samples) {
            if (c_dynamic_on_) {
                if (main_buffer.size() == 1) {
                    const auto main_pointer = main_buffer[0];
                    const auto side_pointer = side_buffer[0];
                    for (size_t i = 0; i < num_samples; ++i) {
                        const auto side_db = zldsp::chore::gainToDecibels(std::abs(side_pointer[i]));
                        auto p = (side_db - low_) * slope_;
                        p = p * p;
                        filter_.template setGain<true>(c_base_gain_ + p * c_gain_diff_);
                        filter_.updateGain();
                        if constexpr (IsBypassed) {
                            filter_.processSample(0, main_pointer[i]);
                        } else {
                            main_pointer[i] = filter_.processSample(0, main_pointer[i]);
                        }
                    }
                } else {
                    for (size_t i = 0; i < num_samples; ++i) {
                        FloatType sum_sqr = 0;
                        for (size_t chan = 0; chan < main_buffer.size(); ++chan) {
                            const auto sample = side_buffer[chan][i];
                            sum_sqr += sample * sample;
                        }
                        const auto side_db = zldsp::chore::squareGainToDecibels(sum_sqr);
                        auto p = (side_db - low_) * slope_;
                        p = p * p;
                        filter_.template setGain<true>(c_base_gain_ + p * c_gain_diff_);
                        filter_.updateGain();
                        for (size_t chan = 0; chan < main_buffer.size(); ++chan) {
                            if constexpr (IsBypassed) {
                                filter_.processSample(chan, main_buffer[chan][i]);
                            } else {
                                main_buffer[chan][i] = filter_.processSample(chan, main_buffer[chan][i]);
                            }
                        }
                    }
                }
            } else {
                filter_.template process<IsBypassed>(main_buffer, num_samples);
            }
        }

        void setDynamicON(const bool f) {
            dynamic_on_.store(f, std::memory_order::relaxed);
        }

        void setTargetGain(const double target_gain) {
            target_gain_.store(target_gain, std::memory_order::relaxed);
        }

        void setThreshold(const FloatType threshold) {
            threshold_.store(threshold, std::memory_order::relaxed);
        }

        void setKnee(const FloatType knee) {
            knee_.store(knee, std::memory_order::relaxed);
        }

    protected:
        IIREmpty &empty_;
        FilterType filter_;
        zldsp::compressor::PSFollower<FloatType, true, false> follower_;

        std::atomic<bool> dynamic_on_{false};
        bool c_dynamic_on_{false};

        std::atomic<double> target_gain_{};
        double c_base_gain_{}, c_gain_diff_{};

        std::atomic<FloatType> threshold_, knee_;
        FloatType low_{}, slope_{};

        void updateThreshold(const FloatType threshold_shift) {
            const auto threshold = threshold_.load(std::memory_order_relaxed) + threshold_shift;
            const auto knee = knee_.load(std::memory_order_relaxed);
            low_ = threshold - knee;
            slope_ = static_cast<FloatType>(0.5) / knee;
        }
    };
}
