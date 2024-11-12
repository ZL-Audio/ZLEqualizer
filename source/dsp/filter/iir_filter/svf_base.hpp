// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef SVF_BASE_HPP
#define SVF_BASE_HPP

namespace zlFilter {
    template<typename SampleType>
    class SVFBase {
    public:
        SVFBase() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            s1.resize(spec.numChannels);
            s2.resize(spec.numChannels);
            reset();
        }

        void reset() {
            std::fill(s1.begin(), s1.end(), static_cast<SampleType>(0));
            std::fill(s2.begin(), s2.end(), static_cast<SampleType>(0));
        }

        void snapToZero() {
            for (auto v: {&s1, &s2}) {
                for (auto &element: *v) {
                    juce::dsp::util::snapToZero(element);
                }
            }
        }

        template<typename ProcessContext>
        void process(const ProcessContext &context) noexcept {
            const auto &inputBlock = context.getInputBlock();
            auto &outputBlock = context.getOutputBlock();
            const auto numChannels = outputBlock.getNumChannels();
            const auto numSamples = outputBlock.getNumSamples();

            jassert(inputBlock.getNumChannels() <= s1.size());
            jassert(inputBlock.getNumChannels() == numChannels);
            jassert(inputBlock.getNumSamples() == numSamples);

            if (context.isBypassed) {
                for (size_t channel = 0; channel < numChannels; ++channel) {
                    auto *inputSamples = inputBlock.getChannelPointer(channel);
                    auto *outputSamples = outputBlock.getChannelPointer(channel);
                    for (size_t i = 0; i < numSamples; ++i) {
                        outputSamples[i] = processSampleBypass(channel, inputSamples[i]);
                    }
                }
            } else {
                for (size_t channel = 0; channel < numChannels; ++channel) {
                    auto *inputSamples = inputBlock.getChannelPointer(channel);
                    auto *outputSamples = outputBlock.getChannelPointer(channel);
                    for (size_t i = 0; i < numSamples; ++i) {
                        outputSamples[i] = processSample(channel, inputSamples[i]);
                    }
                }
            }

#if JUCE_DSP_ENABLE_SNAP_TO_ZERO
            snapToZero();
#endif
        }

        SampleType processSample(const size_t channel, SampleType inputValue) {
            const auto yHP = h * (inputValue - s1[channel] * (g + R2) - s2[channel]);

            const auto yBP = yHP * g + s1[channel];
            s1[channel] = yHP * g + yBP;

            const auto yLP = yBP * g + s2[channel];
            s2[channel] = yBP * g + yLP;

            return chp * yHP + cbp * yBP + clp * yLP;
        }

        SampleType processSampleBypass(const size_t channel, SampleType inputValue) {
            const auto yHP = h * (inputValue - s1[channel] * (g + R2) - s2[channel]);

            const auto yBP = yHP * g + s1[channel];
            s1[channel] = yHP * g + yBP;

            const auto yLP = yBP * g + s2[channel];
            s2[channel] = yBP * g + yLP;

            return yHP - R2 * yBP + yLP;
        }

        void updateFromBiquad(const std::array<double, 6>& coeffs) {
            const auto temp1 = std::sqrt(std::abs((-coeffs[0] - coeffs[1] - coeffs[2])));
            const auto temp2 = std::sqrt(std::abs((-coeffs[0] + coeffs[1] - coeffs[2])));
            g = static_cast<SampleType>(temp1 / temp2);
            R2 = static_cast<SampleType>(2 * (coeffs[0] - coeffs[2]) / (temp1 * temp2));
            h = static_cast<SampleType>(1) / (g * (R2 + g) + static_cast<SampleType>(1));

            chp = static_cast<SampleType>((coeffs[3] - coeffs[4] + coeffs[5]) / (coeffs[0] - coeffs[1] + coeffs[2]));
            cbp = static_cast<SampleType>(2 * (coeffs[5] - coeffs[3]) / (temp1 * temp2));
            clp = static_cast<SampleType>((coeffs[3] + coeffs[4] + coeffs[5]) / (coeffs[0] + coeffs[1] + coeffs[2]));
        }

    private:
        SampleType g, R2, h, chp, cbp, clp;
        std::vector<SampleType> s1, s2;
    };
}

#endif //SVF_BASE_HPP
