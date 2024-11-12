// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef IIR_BASE_HPP
#define IIR_BASE_HPP

#include <numbers>

namespace zlFilter {
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
                if (context.usesSeparateInputAndOutputBlocks()) {
                    outputBlock.copyFrom(inputBlock);
                }
                for (size_t channel = 0; channel < numChannels; ++channel) {
                    auto *inputSamples = inputBlock.getChannelPointer(channel);
                    for (size_t i = 0; i < numSamples; ++i) {
                        processSample(channel, inputSamples[i]);
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
            const auto outputValue = inputValue * mCoeff[0] + s1[channel];
            s1[channel] = (inputValue * mCoeff[1]) - (outputValue * mCoeff[3]) + s2[channel];
            s2[channel] = (inputValue * mCoeff[2]) - (outputValue * mCoeff[4]);
            return outputValue;
        }

        void updateFromBiquad(const std::array<double, 6> &coeff) {
            const auto a0Inv = 1.0 / coeff[0];
            mCoeff[0] = static_cast<SampleType>(coeff[3] * a0Inv);
            mCoeff[1] = static_cast<SampleType>(coeff[4] * a0Inv);
            mCoeff[2] = static_cast<SampleType>(coeff[5] * a0Inv);
            mCoeff[3] = static_cast<SampleType>(coeff[1] * a0Inv);
            mCoeff[4] = static_cast<SampleType>(coeff[2] * a0Inv);
        }

    private:
        std::array<SampleType, 5> mCoeff{0, 0, 0, 0, 0};
        std::vector<SampleType> s1, s2;
    };
}

#endif //IIR_BASE_HPP
