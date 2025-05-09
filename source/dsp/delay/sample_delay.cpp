// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "sample_delay.hpp"

namespace zldsp::delay {
    template<typename FloatType>
    void SampleDelay<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        delay_dsp_.prepare(spec);
        sample_rate_.store(spec.sampleRate);
        delay_samples_.store(static_cast<int>(static_cast<double>(delay_seconds_.load()) * spec.sampleRate));
        to_update_delay_.store(true);
    }

    template<typename FloatType>
    void SampleDelay<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        if (to_update_delay_.exchange(false)) {
            c_delay_samples_ = delay_samples_.load();
            delay_dsp_.setDelay(static_cast<FloatType>(c_delay_samples_));
        }
        if (c_delay_samples_ == 0) { return; }
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        juce::dsp::ProcessContextReplacing<FloatType> context(block);
        delay_dsp_.process(context);
    }

    template<typename FloatType>
    void SampleDelay<FloatType>::process(juce::dsp::AudioBlock<FloatType> block) {
        if (to_update_delay_.exchange(false)) {
            c_delay_samples_ = delay_samples_.load();
            delay_dsp_.setDelay(static_cast<FloatType>(c_delay_samples_));
        }
        if (c_delay_samples_ == 0) { return; }
        juce::dsp::ProcessContextReplacing<FloatType> context(block);
        delay_dsp_.process(context);
    }

    template
    class SampleDelay<float>;

    template
    class SampleDelay<double>;
} // zldsp::delay
