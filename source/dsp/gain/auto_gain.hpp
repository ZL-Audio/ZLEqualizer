// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_AUTO_GAIN_HPP
#define ZLEqualizer_AUTO_GAIN_HPP

#include <juce_dsp/juce_dsp.h>

namespace zlGain {
    /**
     * a lock free, thread safe auto-gain class
     * @tparam FloatType
     */
    template<typename FloatType>
    class AutoGain {
    public:
        AutoGain() = default;

        void prepare(const juce::dsp::ProcessSpec &spec);

        void processPre(juce::AudioBuffer<FloatType> &buffer);

        void processPre(juce::dsp::AudioBlock<FloatType> block);

        void processPost(juce::AudioBuffer<FloatType> &buffer);

        void processPost(juce::dsp::AudioBlock<FloatType> block);

        void setRampDurationSeconds(double newDurationSeconds);

        void enable(bool f);

    private:
        std::atomic<bool> isON{false}, toReset{false}, isPreProcessed{false};
        FloatType preRMS, postRMS;
        juce::dsp::Gain<FloatType> gainDSP;

        FloatType calculateRMS(juce::dsp::AudioBlock<FloatType> block);
    };
} // zlGain

#endif //ZLEqualizer_AUTO_GAIN_HPP
