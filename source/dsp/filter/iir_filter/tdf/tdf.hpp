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
        TDF() : IIR<kFilterSize>() {
        }

        void reset() override {
            for (size_t i = 0; i < this->current_filter_num_; ++i) {
                filters_[i].reset();
            }
        }

        void prepare(const double sample_rate, const size_t num_channels, const size_t) override {
            IIR < kFilterSize > ::prepareSampleRate(sample_rate);
            for (auto& f : filters_) {
                f.prepare(num_channels);
            }
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
            for (size_t filter_idx = 0; filter_idx < this->current_filter_num_; ++filter_idx) {
                sample = filters_[filter_idx].processSample(channel, sample);
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
            this->current_filter_num_ = IIR < kFilterSize > ::updateIIRCoeffs(this->c_filter_type_, this->c_order_,
                                                                              next_freq, this->sample_rate_,
                                                                              next_gain, next_q, this->coeffs_);
            for (size_t i = 0; i < this->current_filter_num_; ++i) {
                filters_[i].updateFromBiquad(this->coeffs_[i]);
            }
        }

        /**
         * update filter coefficients when only gain changes
         */
        void updateGain() override {
            updateCoeffs();
        }

        /**
         * get the array of 2nd order filters
         * @return
         */
        std::array<TDFBase<FloatType>, kFilterSize>& getFilters() {
            return filters_;
        }

    private:
        std::array<TDFBase<FloatType>, kFilterSize> filters_{};
    };
}
