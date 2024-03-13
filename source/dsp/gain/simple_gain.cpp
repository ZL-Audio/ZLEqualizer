// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "simple_gain.hpp"

namespace zlGain {
    template<typename FloatType>
    void Gain<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        gainDSP.prepare(spec);
    }

    template<typename FloatType>
    void Gain<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        process(block);
    }

    template<typename FloatType>
    void Gain<FloatType>::process(juce::dsp::AudioBlock<FloatType> block) {
        if (std::abs(gain.load() - 1) <= FloatType(1e-6)) { return; }
        gainDSP.setGainLinear(gain.load());
        juce::dsp::ProcessContextReplacing<FloatType> context(block);
        gainDSP.process(context);
    }

    template
    class Gain<float>;

    template
    class Gain<double>;
} // zlGain
