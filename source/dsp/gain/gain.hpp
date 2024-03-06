// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_GAIN_HPP
#define ZLEqualizer_GAIN_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlGain {
    template<typename FloatType>
    class Gain {
    public:
        Gain() = default;

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<FloatType> &buffer);

        void process(juce::dsp::AudioBlock<FloatType> block);

        void setGainLinear(const FloatType x) { gain.store(x); }

        void setGainDecibels(const FloatType x) {
            gain.store(juce::Decibels::decibelsToGain(x, FloatType(-240)));
        }

    private:
        std::atomic<FloatType> gain{FloatType(1)};
        juce::dsp::Gain<FloatType> gainDSP;
    };
} // zlGain

#endif //ZLEqualizer_GAIN_HPP
