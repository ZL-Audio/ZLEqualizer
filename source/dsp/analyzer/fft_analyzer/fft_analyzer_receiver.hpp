// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "fft_analyzer_processor.hpp"

namespace zldsp::analyzer {
    namespace hn = hwy::HWY_NAMESPACE;
    /**
     * a fft analyzer receiver which pulls input samples from FIFOs and runs forward FFT
     * @tparam kNum the number of FFTs
     */
    class FFTAnalyzerReceiver {
    public:
        explicit FFTAnalyzerReceiver(FFTAnalyzerProcessor& processor) :
            processor_(processor) {
        }

        /**
         *
         * @param num_channels number of channels
         */
        void prepare(const size_t num_channels) {
            abs_sqr_fft_buffer_.resize(processor_.getFFTSize() / 2 + 1);
            circular_buffer_.resize(num_channels);
            states_.resize(num_channels);
            reset();
        }

        /**
         * reset internal buffers
         */
        void reset() {
            for (size_t chan = 0; chan < circular_buffer_.size(); ++chan) {
                circular_buffer_[chan].resize(processor_.getFFTSize());
                std::ranges::fill(circular_buffer_[chan], 0.f);
            }
            std::ranges::fill(states_, 0.f);
        }

        /**
         * pull data from FIFO into circular buffer
         * @param range
         * @param sample_fifo
         */
        void pull(const zldsp::container::FIFORange range,
                  const std::vector<std::vector<float>>& sample_fifo) {
            const auto num_ready = range.block_size1 + range.block_size2;
            const auto num_replace = static_cast<int>(processor_.getFFTSize()) - num_ready;
            if (!is_on_) { return; }
            for (size_t chan = 0; chan < circular_buffer_.size(); ++chan) {
                auto& circular_buffer{circular_buffer_[chan]};
                auto y = circular_buffer.back();
                auto x = states_[chan];
                std::memmove(circular_buffer.data(),
                             circular_buffer.data() + static_cast<size_t>(num_ready),
                             sizeof(float) * static_cast<size_t>(num_replace));
                if (range.block_size1 > 0) {
                    copyWithHighPass(circular_buffer.data() + static_cast<size_t>(num_replace),
                        sample_fifo[chan].data() + static_cast<size_t>(range.start_index1),
                        static_cast<size_t>(range.block_size1), y, x);
                }
                if (range.block_size2 > 0) {
                    copyWithHighPass(circular_buffer.data() + static_cast<size_t>(num_replace + range.block_size1),
                        sample_fifo[chan].data() + static_cast<size_t>(range.start_index2),
                        static_cast<size_t>(range.block_size2), y, x);
                }
                states_[chan] = x;
            }
        }

        /**
         * run forward FFT to get the absolute square spectrum
         * @param stereo_type
         */
        void forward(const StereoType stereo_type) {
            // run forward FFT & apply tilt
            if (!is_on_) { return; }
            auto& fft_in{processor_.getFFTIn()};
            auto& fft_out{processor_.getFFTOut()};
            const auto& window{processor_.getWindow()};
            if (circular_buffer_.size() != 2 || stereo_type == StereoType::kStereo) {
                for (size_t chan = 0; chan < circular_buffer_.size(); ++chan) {
                    vector::multiply(fft_in.data(), circular_buffer_[chan].data(),
                                     window.data(), window.size());
                    if (chan == 0) {
                        processor_.forwardSqrMag(fft_in.data(), abs_sqr_fft_buffer_.data());
                    } else {
                        processor_.forwardSqrMag(fft_in.data(), fft_out.data());
                        vector::add(abs_sqr_fft_buffer_.data(), fft_out.data(), fft_out.size());
                    }
                }
            } else {
                if (stereo_type == StereoType::kLeft) {
                    vector::multiply(fft_in.data(), circular_buffer_[0].data(),
                                     window.data(), window.size());
                } else if (stereo_type == StereoType::kRight) {
                    vector::multiply(fft_in.data(), circular_buffer_[1].data(),
                                     window.data(), window.size());
                } else if (stereo_type == StereoType::kMid) {
                    static constexpr hn::ScalableTag<float> d;
                    static constexpr size_t lanes = hn::MaxLanes(d);
                    float* __restrict fft_in_ptr = fft_in.data();
                    const float* __restrict in0_ptr = circular_buffer_[0].data();
                    const float* __restrict in1_ptr = circular_buffer_[1].data();
                    const float* __restrict window_ptr = window.data();
                    const auto v_sqrt_over_2 = hn::Set(d, kSqrt2Over2);
                    for (size_t j = 0; j < processor_.getFFTSize(); j += lanes) {
                        const auto v_in0 = hn::LoadU(d, in0_ptr + j);
                        const auto v_in1 = hn::LoadU(d, in1_ptr + j);
                        const auto v_window = hn::LoadU(d, window_ptr + j);
                        const auto v_out = hn::Mul(hn::Add(v_in0, v_in1), v_sqrt_over_2);
                        hn::StoreU(hn::Mul(v_out, v_window), d, fft_in_ptr + j);
                    }
                } else {
                    static constexpr hn::ScalableTag<float> d;
                    static constexpr size_t lanes = hn::MaxLanes(d);
                    float* __restrict fft_in_ptr = fft_in.data();
                    const float* __restrict in0_ptr = circular_buffer_[0].data();
                    const float* __restrict in1_ptr = circular_buffer_[1].data();
                    const float* __restrict window_ptr = window.data();
                    const auto v_sqrt_over_2 = hn::Set(d, kSqrt2Over2);
                    for (size_t j = 0; j < processor_.getFFTSize(); j += lanes) {
                        const auto v_in0 = hn::LoadU(d, in0_ptr + j);
                        const auto v_in1 = hn::LoadU(d, in1_ptr + j);
                        const auto v_window = hn::LoadU(d, window_ptr + j);
                        const auto v_out = hn::Mul(hn::Sub(v_in0, v_in1), v_sqrt_over_2);
                        hn::StoreU(hn::Mul(v_out, v_window), d, fft_in_ptr + j);
                    }
                }
                processor_.forwardSqrMag(fft_in.data(), abs_sqr_fft_buffer_.data());
            }
        }

        void setON(const bool is_on) {
            is_on_ = is_on;
        }

        /**
         * get absolute square spectrum
         * @return
         */
        vector::aligned_vector<float>& getAbsSqrFFTBuffer() {
            return abs_sqr_fft_buffer_;
        }

    protected:
        FFTAnalyzerProcessor& processor_;

        std::vector<vector::aligned_vector<float>> circular_buffer_;
        vector::aligned_vector<float> abs_sqr_fft_buffer_;

        std::vector<float> states_;

        bool is_on_{false};

        static void copyWithHighPass(float* __restrict output, const float* __restrict input,
                                     const size_t num_samples,
                                     float& y, float& x) {
            for (size_t i = 0; i < num_samples; ++i) {
                const auto in = input[i];
                y = in - x + 0.9999f * y;
                output[i] = y;
                x = in;
            }
        }
    };
}
