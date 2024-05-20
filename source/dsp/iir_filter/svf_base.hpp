// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef SVF_BASE_HPP
#define SVF_BASE_HPP

#include "coeff/design_filter.hpp"

namespace zlIIR {
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
            byPassNextBlock.store(true);
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
                outputBlock.copyFrom(inputBlock);
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
            const auto yHP = h * (inputValue - s1[channel] * (g + R2) - s2[channel]);

            const auto yBP = yHP * g + s1[channel];
            s1[channel] = yHP * g + yBP;

            const auto yLP = yBP * g + s2[channel];
            s2[channel] = yBP * g + yLP;

            return chp * yHP + cbp * yBP + clp * yLP;
        }

        void updateFromBiquad(const coeff33 &coeffs) {
            const auto a = std::get<0>(coeffs);
            const auto b = std::get<1>(coeffs);
            const auto temp1 = std::sqrt(std::abs((-a[0] - a[1] - a[2])));
            const auto temp2 = std::sqrt(std::abs((-a[0] + a[1] - a[2])));
            g = static_cast<SampleType>(temp1 / temp2);
            R2 = static_cast<SampleType>(2 * (a[0] - a[2]) / (temp1 * temp2));
            h = static_cast<SampleType>(1) / (g * (R2 + g) + static_cast<SampleType>(1));

            chp = static_cast<SampleType>((b[0] - b[1] + b[2]) / (a[0] - a[1] + a[2]));
            cbp = static_cast<SampleType>(2 * (b[2] - b[0]) / (temp1 * temp2));
            clp = static_cast<SampleType>((b[0] + b[1] + b[2]) / (a[0] + a[1] + a[2]));
        }

    private:
        SampleType g, R2, h, chp, cbp, clp;
        std::vector<SampleType> s1, s2;
        std::atomic<bool> byPassNextBlock{false};
    };
}

#endif //SVF_BASE_HPP
