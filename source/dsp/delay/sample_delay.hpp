// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

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

        void reset() { delayDSP.reset(); }

        void setMaximumDelayInSamples(const int maxDelayInSamples) {
            delayDSP.setMaximumDelayInSamples(maxDelayInSamples);
        }

        void process(juce::AudioBuffer<FloatType> &buffer);

        void process(juce::dsp::AudioBlock<FloatType> block);

        void setDelaySeconds(const FloatType x) {
            delaySeconds.store(x);
            delaySamples.store(static_cast<int>(static_cast<double>(x) * sampleRate.load()));
            toUpdateDelay.store(true);
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
