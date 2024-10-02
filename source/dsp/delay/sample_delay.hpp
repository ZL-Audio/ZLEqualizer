// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_SAMPLE_DELAY_HPP
#define ZLEqualizer_SAMPLE_DELAY_HPP

#include <juce_dsp/juce_dsp.h>

namespace zlDelay {
    /**
     * a lock free, thread safe integer delay class
     * the delay in samples is set to be an integer
     * it will not process the signal if the delay is equal to 0
     * @tparam FloatType
     */
    template<typename FloatType>
    class SampleDelay {
    public:
        SampleDelay() = default;

        void prepare(const juce::dsp::ProcessSpec &spec);

        void setMaximumDelayInSamples(const int maxDelayInSamples) {
            delayDSP.setMaximumDelayInSamples(maxDelayInSamples);
        }

        void process(juce::AudioBuffer<FloatType> &buffer);

        void process(juce::dsp::AudioBlock<FloatType> block);

        void setDelaySeconds(const FloatType x) {
            delaySeconds.store(x);
            delaySamples.store(static_cast<int>(static_cast<double>(x) * sampleRate.load()));
        }

        int getDelaySamples() const {
            return delaySamples.load();
        }

    private:
        std::atomic<double> sampleRate{44100.0};
        std::atomic<FloatType> delaySeconds{0};
        std::atomic<int> delaySamples{0};
        int currentDelaySamples{0};
        std::atomic<bool> toUpdateDelay{false};
        juce::dsp::DelayLine<FloatType> delayDSP;
    };
} // zlDelay

#endif //ZLEqualizer_SAMPLE_DELAY_HPP
