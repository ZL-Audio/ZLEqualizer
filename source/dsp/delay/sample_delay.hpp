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

namespace zldsp::delay {
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

        void reset() { delay_dsp_.reset(); }

        void setMaximumDelayInSamples(const int maxDelayInSamples) {
            delay_dsp_.setMaximumDelayInSamples(maxDelayInSamples);
        }

        void process(juce::AudioBuffer<FloatType> &buffer);

        void process(juce::dsp::AudioBlock<FloatType> block);

        void setDelaySeconds(const FloatType x) {
            delay_seconds_.store(x);
            delay_samples_.store(static_cast<int>(static_cast<double>(x) * sample_rate_.load()));
            to_update_delay_.store(true);
        }

        int getDelaySamples() const {
            return delay_samples_.load();
        }

    private:
        std::atomic<double> sample_rate_{44100.0};
        std::atomic<FloatType> delay_seconds_{0};
        std::atomic<int> delay_samples_{0};
        int c_delay_samples_{0};
        std::atomic<bool> to_update_delay_{false};
        juce::dsp::DelayLine<FloatType, juce::dsp::DelayLineInterpolationTypes::None> delay_dsp_;
    };
} // zldsp::delay
