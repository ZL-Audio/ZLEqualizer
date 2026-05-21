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

#include "../../../vector/vector.hpp"
#include "../iir/iir.hpp"
#include "../tdf/tdf_base.hpp"

namespace zldsp::filter {
    /**
     * an IIR filter which processes audio on the real-time thread
     * @tparam FloatType the float type of input audio buffer
     * @tparam kFilterSize the number of cascading filters
     */
    template <typename FloatType, size_t kFilterSize>
    class Parallel final : public IIR<kFilterSize> {
    public:
        Parallel() :
            IIR<kFilterSize>() {
        }

        void reset() override {
            for (auto& s : s1s_) {
                std::ranges::fill(s.begin(), s.end(), static_cast<FloatType>(0));
            }
            for (auto& s : s2s_) {
                std::ranges::fill(s.begin(), s.end(), static_cast<FloatType>(0));
            }
        }

        void prepare(const double sample_rate, const size_t num_channels, const size_t max_num_samples) override {
            IIR<kFilterSize>::prepareSampleRate(sample_rate);
            for (auto& s : s1s_) {
                s.resize(num_channels);
            }
            for (auto& s : s2s_) {
                s.resize(num_channels);
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
        template <bool bypass = false>
        void process(std::span<FloatType*> buffer, const size_t num_samples) {
            if (this->c_freq_.isSmoothing() || this->c_gain_.isSmoothing() || this->c_q_.isSmoothing()) {
                if (should_be_parallel_) {
                    processParallel<bypass, true, true>(buffer, num_samples);
                } else {
                    processParallel<bypass, true, false>(buffer, num_samples);
                }
            } else {
                if (should_be_parallel_) {
                    processParallel<bypass, false, true>(buffer, num_samples);
                } else {
                    processParallel<bypass, false, false>(buffer, num_samples);
                }
            }
        }

        template <bool bypass = false, bool smooth = false, bool is_parallel = false>
        void processParallel(std::span<FloatType*> buffer, const size_t num_samples) {
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
                        if constexpr (is_parallel) {
                            buffer[channel][i] = processSample(channel, buffer[channel][i]) * parallel_multiplier_;
                        } else {
                            buffer[channel][i] = processSample(channel, buffer[channel][i]);
                        }
                    }
                }
            }
        }

        FloatType processSample(const size_t channel, FloatType sample) {
            for (size_t filter_idx = 0; filter_idx < this->current_filter_num_; ++filter_idx) {
                const auto& coeff{IIR<kFilterSize>::coeffs_[filter_idx]};
                auto& s1{s1s_[filter_idx][channel]};
                auto& s2{s2s_[filter_idx][channel]};
                const auto output = sample * coeff[2] + s1;
                s1 = (sample * coeff[3]) - (output * coeff[0]) + s2;
                s2 = (sample * coeff[4]) - (output * coeff[1]);
                sample = output;
            }
            return sample;
        }

        /**
         * add the parallel buffer
         * @param buffer
         * @param num_samples
         */
        template <bool bypass = false>
        void processPost(std::span<FloatType*> buffer, const size_t num_samples) {
            if constexpr (!bypass) {
                if (should_be_parallel_) {
                    for (size_t i = 0; i < buffer.size(); ++i) {
                        vector::add(buffer[i], parallel_buffers_pointers_[i], num_samples);
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
                parallel_multiplier_ = static_cast<FloatType>(dbToGain(this->c_gain_.getCurrent()) - 1.0);
            } else if (this->c_filter_type_ == kLowShelf && this->c_order_ <= 2) {
                should_be_parallel_ = true;
                this->current_filter_num_ = IIR<kFilterSize>::updateIIRCoeffs(FilterType::kLowPass, this->c_order_,
                                                                              next_freq, this->sample_rate_,
                                                                              next_gain, next_q, this->coeffs_);
                parallel_multiplier_ = static_cast<FloatType>(dbToGain(this->c_gain_.getCurrent()) - 1.0);
            } else if (this->c_filter_type_ == kHighShelf && this->c_order_ <= 2) {
                should_be_parallel_ = true;
                this->current_filter_num_ = IIR<kFilterSize>::updateIIRCoeffs(kHighPass, this->c_order_,
                                                                              next_freq, this->sample_rate_,
                                                                              next_gain, next_q, this->coeffs_);
                parallel_multiplier_ = static_cast<FloatType>(dbToGain(this->c_gain_.getCurrent()) - 1.0);
            } else {
                should_be_parallel_ = false;
                this->current_filter_num_ = IIR<kFilterSize>::updateIIRCoeffs(this->c_filter_type_, this->c_order_,
                                                                              next_freq, this->sample_rate_,
                                                                              next_gain, next_q, this->coeffs_);
            }
        }

        void updateGainLinear(const FloatType g_linear_sqrt) {
            if (should_be_parallel_) {
                parallel_multiplier_ = static_cast<FloatType>(g_linear_sqrt * g_linear_sqrt - 1.0);
            } else {
                FilterDesign::updateGainLinear<IvantsovCoeff>(this->c_filter_type_, this->c_order_,
                                                              g_linear_sqrt, this->cache_.data(), this->coeffs_);
            }
        }

        [[nodiscard]] bool getShouldBeParallel() const {
            return should_be_parallel_;
        }

        std::span<FloatType*> getParallelBuffer() {
            return parallel_buffers_pointers_;
        }

        FloatType getMultiplier() const {
            return parallel_multiplier_;
        }

    private:
        std::array<std::vector<FloatType>, kFilterSize> s1s_{}, s2s_{};
        std::vector<std::vector<FloatType>> parallel_buffers_;
        std::vector<FloatType*> parallel_buffers_pointers_;
        bool should_be_parallel_ = false;
        FloatType parallel_multiplier_{};
    };
}
