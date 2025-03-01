// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_LOUDNESS_K_WEIGHTING_FILTER_HPP
#define ZL_LOUDNESS_K_WEIGHTING_FILTER_HPP

#include "../filter/filter.hpp"

namespace zlLoudness {
    template<typename FloatType>
    class KWeightingFilter {
    public:
        KWeightingFilter() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            highPassF.prepare(spec);
            highShelfF.prepare(spec);
            const auto w1 = zlFilter::ppi * 38.13713296248405 / spec.sampleRate;
            const auto w2 = zlFilter::ppi * 1500.6868667368922 / spec.sampleRate;
            highPassF.updateFromBiquad(zlFilter::MartinCoeff::get2HighPass(
                w1, 0.500242812458813));
            highShelfF.updateFromBiquad(zlFilter::MartinCoeff::get2HighShelf(
                w2, 1.5847768458311522, 0.7096433028107384));
        }

        void reset() {
            highPassF.reset();
            highShelfF.reset();
        }

        void process(juce::AudioBuffer<FloatType> &buffer) {
            const auto writerPointer = buffer.getArrayOfWritePointers();
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                    auto sample = *(writerPointer[channel] + i);
                    sample = highPassF.processSample(channel, sample);
                    sample = highShelfF.processSample(channel, sample);
                    *(writerPointer[channel] + i) = sample;
                }
            }
            buffer.applyGain(bias);
        }

    private:
        zlFilter::IIRBase<FloatType> highPassF, highShelfF;
        static constexpr FloatType bias = 1.0051643348917434;
    };
}

#endif //ZL_LOUDNESS_K_WEIGHTING_FILTER_HPP
