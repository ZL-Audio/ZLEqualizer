// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../filter/filter.hpp"

namespace zldsp::loudness {
    template<typename FloatType, bool UseLowPass = false>
    class KWeightingFilter {
    public:
        KWeightingFilter() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            high_pass_.prepare(spec);
            high_shelf_.prepare(spec);
            const auto w1 = zldsp::filter::ppi * 38.13713296248405 / spec.sampleRate;
            const auto w2 = zldsp::filter::ppi * 1500.6868667368922 / spec.sampleRate;
            high_pass_.updateFromBiquad(zldsp::filter::MartinCoeff::get2HighPass(
                w1, 0.500242812458813));
            high_shelf_.updateFromBiquad(zldsp::filter::MartinCoeff::get2HighShelf(
                w2, 1.5847768458311522, 0.7096433028107384));
            if (UseLowPass) {
                low_pass_.prepare(spec);
                const auto w3 = zldsp::filter::ppi * 22000.0 / spec.sampleRate;
                low_pass_.updateFromBiquad(zldsp::filter::MartinCoeff::get2LowPass(
                    w3, 0.7071067811865476));
            }
        }

        void reset() {
            high_pass_.reset();
            high_shelf_.reset();
            if (UseLowPass) {
                low_pass_.reset();
            }
        }

        void process(juce::AudioBuffer<FloatType> &buffer) {
            const auto writer_pointer = buffer.getArrayOfWritePointers();
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                    auto sample = *(writer_pointer[channel] + i);
                    sample = high_pass_.processSample(static_cast<size_t>(channel), sample);
                    sample = high_shelf_.processSample(static_cast<size_t>(channel), sample);
                    if (UseLowPass) {
                        sample = low_pass_.processSample(static_cast<size_t>(channel), sample);
                    }
                    *(writer_pointer[channel] + i) = sample;
                }
            }
            buffer.applyGain(kBias);
        }

    private:
        zldsp::filter::IIRBase<FloatType> high_pass_, high_shelf_, low_pass_;
        static constexpr FloatType kBias = FloatType(1.0051643348917434);
    };
}
