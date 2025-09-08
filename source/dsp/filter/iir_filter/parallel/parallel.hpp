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

#include "../../../vector/vector.hpp"
#include "../../filter_design/filter_design.hpp"
#include "../coeff/martin_coeff.hpp"
#include "../../empty_filter/empty.hpp"
#include "../iir/iir.hpp"
#include "../tdf/tdf_base.hpp"

namespace zldsp::filter {
    /**
     * an IIR filter which processes audio on the real-time thread
     * @tparam FloatType the float type of input audio buffer
     * @tparam kFilterSize the number of cascading filters
     */
    template<typename FloatType, size_t kFilterSize>
    class Parallel final : public IIR<kFilterSize> {
    public:
        Parallel() : IIR<kFilterSize>() {
        }

        void reset() override {
            for (size_t i = 0; i < this->current_filter_num_; ++i) {
                filters_[i].reset();
            }
        }

        void prepare(const double sample_rate, const size_t num_channels, const size_t max_num_samples) override {
            IIR<kFilterSize>::prepareSampleRate(sample_rate);
            for (auto &f: filters_) {
                f.prepare(num_channels);
            }
            parallel_buffers_.resize(num_channels);
            parallel_buffers_pointers_.resize(num_channels);
            for (size_t i = 0; i < num_channels; ++i) {
                parallel_buffers_[i].resize(max_num_samples);
                parallel_buffers_pointers_[i] = parallel_buffers_[i].data();
            }
        }

        /**
         * process the incoming audio buffer
         * @param buffer
         * @param num_samples
         */
        template<bool bypass = false>
        void process(std::span<FloatType *> buffer, const size_t num_samples) {
            if (this->c_freq_.isSmoothing() || this->c_gain_.isSmoothing() || this->c_q_.isSmoothing()) {
                if (should_be_parallel_) {
                    parallel_buffers_pointers_.clear();
                    for (size_t chan = 0; chan < buffer.size(); ++chan) {
                        parallel_buffers_pointers_.emplace_back(parallel_buffers_[chan].data());
                        zldsp::vector::copy(parallel_buffers_[chan].data(), buffer[chan], num_samples);
                    }
                    processParallel<bypass, true>(parallel_buffers_pointers_, num_samples);
                } else {
                    processParallel<bypass, true>(buffer, num_samples);
                }
            } else {
                if (should_be_parallel_) {
                    parallel_buffers_pointers_.clear();
                    for (size_t chan = 0; chan < buffer.size(); ++chan) {
                        parallel_buffers_pointers_.emplace_back(parallel_buffers_[chan].data());
                        zldsp::vector::copy(parallel_buffers_[chan].data(), buffer[chan], num_samples);
                    }
                    processParallel<bypass, false>(parallel_buffers_pointers_, num_samples);
                } else {
                    processParallel<bypass, false>(buffer, num_samples);
                }
            }
        }

        template<bool bypass = false, bool smooth = false>
        void processParallel(std::span<FloatType *> buffer, const size_t num_samples) {
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
         * add the parallel buffer
         * @param buffer
         * @param num_samples
         */
        template<bool bypass = false>
        void processPost(std::span<FloatType *> buffer, const size_t num_samples) {
            if constexpr (!bypass) {
                if (should_be_parallel_) {
                    for (size_t i = 0; i < buffer.size(); ++i) {
                        auto v1 = kfr::make_univector(buffer[i], num_samples);
                        auto v2 = kfr::make_univector(parallel_buffers_pointers_[i], num_samples);
                        v1 = v1 + v2 * parallel_multiplier_;
                    }
                }
            }
        }

        /**
         * update filter coefficients
         */
        void updateCoeffs() override {
            const auto next_freq = this->c_freq_.getCurrent();
            const auto next_gain = this->c_gain_.getCurrent();
            const auto next_q = this->c_q_.getCurrent();
            if (this->c_filter_type_ == kPeak && this->c_order_ <= 4) {
                should_be_parallel_ = true;
                this->current_filter_num_ = this->updateIIRCoeffs(FilterType::kBandPass, this->c_order_,
                                                                  next_freq, this->sample_rate_,
                                                                  next_gain, next_q, this->coeffs_);
                updateGain();
            } else if (this->c_filter_type_ == kLowShelf && this->c_order_ <= 2) {
                should_be_parallel_ = true;
                this->current_filter_num_ = IIR<kFilterSize>::updateIIRCoeffs(FilterType::kLowPass, this->c_order_,
                                                                              next_freq, this->sample_rate_,
                                                                              next_gain, next_q, this->coeffs_);
                updateGain();
            } else if (this->c_filter_type_ == kHighShelf && this->c_order_ <= 2) {
                should_be_parallel_ = true;
                this->current_filter_num_ = IIR<kFilterSize>::updateIIRCoeffs(kHighPass, this->c_order_,
                                                                              next_freq, this->sample_rate_,
                                                                              next_gain, next_q, this->coeffs_);
                updateGain();
            } else {
                should_be_parallel_ = false;
                this->current_filter_num_ = IIR<kFilterSize>::updateIIRCoeffs(this->c_filter_type_, this->c_order_,
                                                                              next_freq, this->sample_rate_,
                                                                              next_gain, next_q, this->coeffs_);
            }
            for (size_t i = 0; i < this->current_filter_num_; ++i) {
                filters_[i].updateFromBiquad(this->coeffs_[i]);
            }
        }

        /**
         * update filter coefficients when only gain changes
         * for parallel filters, only multiplier needs to be updated
         */
        void updateGain() override {
            if (should_be_parallel_) {
                parallel_multiplier_ = static_cast<FloatType>(dbToGain(this->c_gain_.getCurrent()) - 1.0);
            } else {
                updateCoeffs();
            }
        }

        /**
         * get the array of 2nd order filters
         * @return
         */
        std::array<TDFBase<FloatType>, kFilterSize> &getFilters() {
            return filters_;
        }

        [[nodiscard]] bool getShouldBeParallel() const {
            return should_be_parallel_;
        }

        std::span<FloatType *> getParallelBuffer() {
            return parallel_buffers_pointers_;
        }

    private:
        std::array<TDFBase<FloatType>, kFilterSize> filters_{};
        std::vector<std::vector<FloatType> > parallel_buffers_;
        std::vector<FloatType *> parallel_buffers_pointers_;
        bool should_be_parallel_ = false;
        FloatType parallel_multiplier_{};
    };
}
