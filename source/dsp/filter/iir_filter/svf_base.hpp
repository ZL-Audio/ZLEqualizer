// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along_ with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

namespace zldsp::filter {
    template<typename SampleType>
    class SVFBase {
    public:
        SVFBase() = default;

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
                for (size_t channel = 0; channel < num_channels; ++channel) {
                    auto *input_samples = input_block.getChannelPointer(channel);
                    auto *output_samples = output_block.getChannelPointer(channel);
                    for (size_t i = 0; i < num_samples; ++i) {
                        output_samples[i] = processSampleBypass(channel, input_samples[i]);
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

        SampleType processSample(const size_t channel, SampleType input_value) {
            const auto y_hp = h_ * (input_value - s1_[channel] * (g_ + R2_) - s2_[channel]);

            const auto y_bp = y_hp * g_ + s1_[channel];
            s1_[channel] = y_hp * g_ + y_bp;

            const auto y_lp = y_bp * g_ + s2_[channel];
            s2_[channel] = y_bp * g_ + y_lp;

            return chp_ * y_hp + cbp_ * y_bp + clp_ * y_lp;
        }

        SampleType processSampleBypass(const size_t channel, SampleType input_value) {
            const auto y_hp = h_ * (input_value - s1_[channel] * (g_ + R2_) - s2_[channel]);

            const auto y_bp = y_hp * g_ + s1_[channel];
            s1_[channel] = y_hp * g_ + y_bp;

            const auto y_lp = y_bp * g_ + s2_[channel];
            s2_[channel] = y_bp * g_ + y_lp;

            return y_hp - R2_ * y_bp + y_lp;
        }

        void updateFromBiquad(const std::array<double, 6> &coeffs) {
            const auto temp1 = std::sqrt(std::abs((-coeffs[0] - coeffs[1] - coeffs[2])));
            const auto temp2 = std::sqrt(std::abs((-coeffs[0] + coeffs[1] - coeffs[2])));
            g_ = static_cast<SampleType>(temp1 / temp2);
            R2_ = static_cast<SampleType>(2 * (coeffs[0] - coeffs[2]) / (temp1 * temp2));
            h_ = static_cast<SampleType>(1) / (g_ * (R2_ + g_) + static_cast<SampleType>(1));

            chp_ = static_cast<SampleType>((coeffs[3] - coeffs[4] + coeffs[5]) / (coeffs[0] - coeffs[1] + coeffs[2]));
            cbp_ = static_cast<SampleType>(2 * (coeffs[5] - coeffs[3]) / (temp1 * temp2));
            clp_ = static_cast<SampleType>((coeffs[3] + coeffs[4] + coeffs[5]) / (coeffs[0] + coeffs[1] + coeffs[2]));
        }

    private:
        SampleType g_, R2_, h_, chp_, cbp_, clp_;
        std::vector<SampleType> s1_, s2_;
    };
}
