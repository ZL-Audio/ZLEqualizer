// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_dsp/juce_dsp.h>

#include "../../fft/fft.hpp"
#include "../../vector/vector.hpp"

namespace zldsp::filter {
    template<typename FloatType, size_t DefaultFFTOrder = 10>
    class FIRBase {
    public:
        virtual ~FIRBase() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            if (spec.sampleRate <= 50000) {
                setOrder(static_cast<size_t>(spec.numChannels), DefaultFFTOrder);
            } else if (spec.sampleRate <= 100000) {
                setOrder(static_cast<size_t>(spec.numChannels), DefaultFFTOrder + 1);
            } else if (spec.sampleRate <= 200000) {
                setOrder(static_cast<size_t>(spec.numChannels), DefaultFFTOrder + 2);
            } else {
                setOrder(static_cast<size_t>(spec.numChannels), DefaultFFTOrder + 3);
            }
        }

        void reset() {
            pos_ = 0;
            count_ = 0;
            for (auto &fifo: input_fifo_) {
                fifo.resize(fft_size_);
                std::fill(fifo.begin(), fifo.end(), 0.f);
            }
            for (auto &fifo: output_fifo_) {
                fifo.resize(fft_size_);
                std::fill(fifo.begin(), fifo.end(), 0.f);
            }
            std::fill(fft_in_.begin(), fft_in_.end(), 0.f);
        }

        template<bool isBypassed = false>
        void process(juce::AudioBuffer<FloatType> &buffer) {
            for (size_t i = 0; i < static_cast<size_t>(buffer.getNumSamples()); ++i) {
                for (size_t channel = 0; channel < static_cast<size_t>(buffer.getNumChannels()); ++channel) {
                    auto write_pointer = buffer.getWritePointer(static_cast<int>(channel), static_cast<int>(i));
                    input_fifo_[channel][pos_] = static_cast<float>(*write_pointer);
                    *write_pointer = static_cast<FloatType>(output_fifo_[channel][pos_]);
                    output_fifo_[channel][pos_] = FloatType(0);
                }

                pos_ += 1;
                if (pos_ == fft_size_) {
                    pos_ = 0;
                }
                count_ += 1;
                if (count_ == hop_size_) {
                    count_ = 0;
                    processFrame<isBypassed>();
                }
            }
        }

        int getLatency() const { return latency_.load(); }

    protected:
        zldsp::fft::KFREngine<float> fft_;
        zldsp::fft::WindowFunction<float> window1_, window2_;

        size_t fft_order_ = DefaultFFTOrder;
        size_t fft_size_ = static_cast<size_t>(1) << fft_order_;
        size_t num_bins_ = fft_size_ / 2 + 1;
        size_t overlap_ = 4; // 75% overlap
        size_t hop_size_ = fft_size_ / overlap_;
        static constexpr float kWindowCorrection = 2.0f / 3.0f;
        static constexpr float kBypassCorrection = 1.0f / 4.0f;
        // counts up until the next hop.
        size_t count_ = 0;
        // write position in input FIFO and read position in output FIFO.
        size_t pos_ = 0;
        // circular buffers for incoming and outgoing audio data.
        std::vector<kfr::univector<float> > input_fifo_, output_fifo_;
        // circular FFT working space which contains interleaved complex numbers.
        kfr::univector<float> fft_in_, fft_data_;

        size_t fft_data_pos_ = 0;
        std::atomic<int> latency_{0};


        void setFFTOrder(const size_t channel_num, const size_t order) {
            fft_order_ = order;
            fft_size_ = static_cast<size_t>(1) << fft_order_;
            num_bins_ = fft_size_ / 2 + 1;
            hop_size_ = fft_size_ / overlap_;
            latency_.store(static_cast<int>(fft_size_));

            fft_.setOrder(fft_order_);
            window1_.setWindow(fft_size_, juce::dsp::WindowingFunction<float>::WindowingMethod::hann,
                1.f / static_cast<float>(fft_size_), false, true);
            window2_.setWindow(fft_size_, juce::dsp::WindowingFunction<float>::WindowingMethod::hann,
                kWindowCorrection, false, true);

            input_fifo_.resize(channel_num);
            output_fifo_.resize(channel_num);
            fft_in_.resize(fft_size_);
            fft_data_.resize(fft_size_ * 2);
        }

        template<bool isBypassed = false>
        void processFrame() {
            for (size_t idx = 0; idx < input_fifo_.size(); ++idx) {

                // Copy the input FIFO into the FFT working space in two parts.
                zldsp::vector::copy(fft_in_.data(), input_fifo_[idx].data() + pos_, fft_size_ - pos_);
                if (pos_ > 0) {
                    zldsp::vector::copy(fft_in_.data() + fft_size_ - pos_, input_fifo_[idx].data(), pos_);
                }

                if (!isBypassed) {
                    window1_.multiply(fft_in_);

                    fft_.forward(fft_in_.data(), fft_data_.data());
                    processSpectrum();
                    fft_.backward(fft_data_.data(), fft_in_.data());

                    window2_.multiply(fft_in_);
                } else {
                    fft_in_ = fft_in_ * kBypassCorrection;
                }

                for (size_t i = 0; i < pos_; ++i) {
                    output_fifo_[idx][i] += fft_in_[i + fft_size_ - pos_];
                }
                for (size_t i = 0; i < fft_size_ - pos_; ++i) {
                    output_fifo_[idx][i + pos_] += fft_in_[i];
                }
            }
        }

        virtual void setOrder(size_t channelNum, size_t order) = 0;

        virtual void processSpectrum() = 0;
    };
}
