// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include <vector>
#include <algorithm>

#include "tdf_base.hpp"
#include "../iir/iir.hpp"

namespace zldsp::filter {
    /**
     * an IIR filter which processes audio on the real-time thread
     * @tparam FloatType the float type of input audio buffer
     * @tparam kFilterSize the number of cascading filters
     */
    template <typename FloatType, size_t kFilterSize>
    class TDF final : public IIR<kFilterSize> {
    public:
        TDF() :
            IIR<kFilterSize>() {
        }

        void reset() override {
            std::ranges::fill(s1s_, static_cast<FloatType>(0));
            std::ranges::fill(s2s_, static_cast<FloatType>(0));
        }

        void prepare(const double sample_rate, const size_t num_channels, const size_t) override {
            IIR<kFilterSize>::prepareSampleRate(sample_rate);
            s1s_.assign(num_channels * kFilterSize, static_cast<FloatType>(0));
            s2s_.assign(num_channels * kFilterSize, static_cast<FloatType>(0));
        }

        /**
         * process the incoming audio buffer
         * @param buffer
         * @param num_samples
         */
        template <bool bypass = false>
        void process(std::span<FloatType*> buffer, const size_t num_samples) {
            if (this->c_freq_.isSmoothing() || this->c_gain_.isSmoothing() || this->c_q_.isSmoothing()) {
                processTDF<bypass, true>(buffer, num_samples);
            } else {
                processTDF<bypass, false>(buffer, num_samples);
            }
        }

        template <bool bypass = false, bool smooth = false>
        void processTDF(std::span<FloatType*> buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                if constexpr (smooth) {
                    this->c_freq_.getNext();
                    this->c_gain_.getNext();
                    this->c_q_.getNext();
                    updateCoeffs();
                }
                for (size_t channel = 0; channel < buffer.size(); ++channel) {
                    if constexpr (bypass) {
                        processSample(channel, buffer[channel][i]);
                    } else {
                        buffer[channel][i] = processSample(channel, buffer[channel][i]);
                    }
                }
            }
        }

        FloatType processSample(const size_t channel, FloatType sample) {
            const size_t channel_offset = channel * kFilterSize;
            for (size_t filter_idx = 0; filter_idx < this->current_filter_num_; ++filter_idx) {
                const auto& coeff{IIR<kFilterSize>::coeffs_[filter_idx]};
                auto& s1{s1s_[channel_offset + filter_idx]};
                auto& s2{s2s_[channel_offset + filter_idx]};
                const auto output = sample * coeff[2] + s1;
                s1 = (sample * coeff[3]) - (output * coeff[0]) + s2;
                s2 = (sample * coeff[4]) - (output * coeff[1]);
                sample = output;
            }
            return sample;
        }

        /**
         * update filter coefficients
         */
        void updateCoeffs() override {
            const auto next_freq = this->c_freq_.getCurrent();
            const auto next_gain = this->c_gain_.getCurrent();
            const auto next_q = this->c_q_.getCurrent();
            this->current_filter_num_ = IIR<kFilterSize>::updateIIRCoeffs(this->c_filter_type_, this->c_order_,
                                                                          next_freq, this->sample_rate_,
                                                                          next_gain, next_q, this->coeffs_);
        }

        void updateGainLinear(const FloatType g_linear_sqrt) {
            FilterDesign::updateGainLinear<IvantsovCoeff>(this->c_filter_type_, this->c_order_,
                                                          g_linear_sqrt, this->cache_.data(), this->coeffs_);
        }

    private:
        std::vector<FloatType> s1s_{};
        std::vector<FloatType> s2s_{};
    };
}
