// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "origin_gain.hpp"

namespace zldsp::gain {
    /**
     * a lock free, thread safe gain class
     * it will not process the signal if the gain is equal to 1
     * @tparam FloatType
     */
    template<typename FloatType>
    class SimpleGain {
    public:
        SimpleGain() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            gain_dsp_.prepare(spec, 1.0);
        }

        template<bool IsBypassed = false>
        void process(juce::AudioBuffer<FloatType> &buffer) {
            if (IsBypassed) { return; }
            if (std::abs(gain_.load() - 1) <= FloatType(1e-6)) { return; }
            gain_dsp_.setGainLinear(gain_.load());
            gain_dsp_.template process<false>(buffer);
        }

        template<bool isBypassed = false>
        void process(juce::dsp::AudioBlock<FloatType> block) {
            if (isBypassed) { return; }
            if (std::abs(gain_.load() - 1) <= FloatType(1e-6)) { return; }
            gain_dsp_.setGainLinear(gain_.load());
            gain_dsp_.template process<false>(block);
        }

        void setGainLinear(const FloatType x) { gain_.store(x); }

        void setGainDecibels(const FloatType x) {
            gain_.store(juce::Decibels::decibelsToGain(x, FloatType(-240)));
        }

        FloatType getGainDecibels() const {
            return juce::Decibels::gainToDecibels(gain_.load());
        }

    private:
        std::atomic<FloatType> gain_{FloatType(1)};
        Gain<FloatType> gain_dsp_;
    };
} // zldsp::gain
