// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <numbers>

namespace zldsp::filter {
    template<typename SampleType>
    class IIRBase {
    public:
        // w should be an array of std::exp(-2pi * f / samplerate * i)
        static void updateResponse(
            const std::array<double, 6> &coeff,
            const std::vector<std::complex<SampleType> > &wis, std::vector<std::complex<SampleType> > &response) {
            for (size_t idx = 0; idx < wis.size(); ++idx) {
                response[idx] *= getResponse(coeff, wis[idx]);
            }
        }

        // w should be std::exp(-2pi * f / samplerate * i)
        // const auto wi = std::exp(std::complex<SampleType>(SampleType(0), w))
        static std::complex<SampleType> getResponse(const std::array<double, 6> &coeff,
                                                    const std::complex<SampleType> &wi) {
            const auto wi2 = wi * wi;
            return (static_cast<SampleType>(coeff[3]) +
                    static_cast<SampleType>(coeff[4]) * wi +
                    static_cast<SampleType>(coeff[5]) * wi2) / (
                       static_cast<SampleType>(coeff[0]) +
                       static_cast<SampleType>(coeff[1]) * wi +
                       static_cast<SampleType>(coeff[2]) * wi2);
        }

        IIRBase() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            s1_.resize(spec.numChannels);
            s2_.resize(spec.numChannels);
            reset();
        }

        void reset() {
            std::fill(s1_.begin(), s1_.end(), static_cast<SampleType>(0));
            std::fill(s2_.begin(), s2_.end(), static_cast<SampleType>(0));
        }

        void snapToZero() {
            for (auto v: {&s1_, &s2_}) {
                for (auto &element: *v) {
                    juce::dsp::util::snapToZero(element);
                }
            }
        }

        template<typename ProcessContext>
        void process(const ProcessContext &context) noexcept {
            const auto &input_block = context.getInputBlock();
            auto &output_block = context.getOutputBlock();
            const auto num_channels = output_block.getNumChannels();
            const auto num_samples = output_block.getNumSamples();

            jassert(input_block.getNumChannels() <= s1_.size());
            jassert(input_block.getNumChannels() == num_channels);
            jassert(input_block.getNumSamples() == num_samples);

            if (context.isBypassed) {
                if (context.usesSeparateInputAndOutputBlocks()) {
                    output_block.copyFrom(input_block);
                }
                for (size_t channel = 0; channel < num_channels; ++channel) {
                    auto *input_samples = input_block.getChannelPointer(channel);
                    for (size_t i = 0; i < num_samples; ++i) {
                        processSample(channel, input_samples[i]);
                    }
                }
            } else {
                for (size_t channel = 0; channel < num_channels; ++channel) {
                    auto *input_samples = input_block.getChannelPointer(channel);
                    auto *output_samples = output_block.getChannelPointer(channel);
                    for (size_t i = 0; i < num_samples; ++i) {
                        output_samples[i] = processSample(channel, input_samples[i]);
                    }
                }
            }

#if JUCE_DSP_ENABLE_SNAP_TO_ZERO
            snapToZero();
#endif
        }

        SampleType processSample(const size_t channel, SampleType inputValue) {
            const auto outputValue = inputValue * coeff_[0] + s1_[channel];
            s1_[channel] = (inputValue * coeff_[1]) - (outputValue * coeff_[3]) + s2_[channel];
            s2_[channel] = (inputValue * coeff_[2]) - (outputValue * coeff_[4]);
            return outputValue;
        }

        void updateFromBiquad(const std::array<double, 6> &coeff) {
            const auto a0_inv = 1.0 / coeff[0];
            coeff_[0] = static_cast<SampleType>(coeff[3] * a0_inv);
            coeff_[1] = static_cast<SampleType>(coeff[4] * a0_inv);
            coeff_[2] = static_cast<SampleType>(coeff[5] * a0_inv);
            coeff_[3] = static_cast<SampleType>(coeff[1] * a0_inv);
            coeff_[4] = static_cast<SampleType>(coeff[2] * a0_inv);
        }

    private:
        std::array<SampleType, 5> coeff_{0, 0, 0, 0, 0};
        std::vector<SampleType> s1_, s2_;
    };
}
