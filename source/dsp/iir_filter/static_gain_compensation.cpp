// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "static_gain_compensation.hpp"

namespace zlIIR {
    template<typename FloatType>
    StaticGainCompensation<FloatType>::StaticGainCompensation(Filter<FloatType> &filter)
        : target(filter) {
    }

    template<typename FloatType>
    void StaticGainCompensation<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        gain.prepare(spec);
    }

    template<typename FloatType>
    void StaticGainCompensation<FloatType>::update() {
        if (!isON.load()) { return; }
        switch (target.getFilterType()) {
            case peak: {
                const auto f = target.getFreq();
                const auto g = juce::jlimit(FloatType(-12), FloatType(12), target.getGain());
                const auto q = juce::jlimit(FloatType(0.1), FloatType(5), target.getQ());
                gain.setGainDecibels(getPeakEstimation(f, g, q));
                break;
            }
            case lowShelf: {
                const auto f = target.getFreq();
                const auto g = juce::jlimit(FloatType(-12), FloatType(12), target.getGain());
                gain.setGainDecibels(getLowShelfEstimation(f, g));
                break;
            }
            case highShelf: {
                const auto f = target.getFreq();
                const auto g = juce::jlimit(FloatType(-12), FloatType(12), target.getGain());
                gain.setGainDecibels(getHighShelfEstimation(f, g));
                break;
            }
            case tiltShelf:
            case bandShelf:
            case lowPass:
            case highPass:
            case notch:
            case bandPass:
            default:
                break;
        }
    }

    template<typename FloatType>
    void StaticGainCompensation<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        process(juce::dsp::AudioBlock<FloatType>(buffer));
    }

    template<typename FloatType>
    void StaticGainCompensation<FloatType>::process(juce::dsp::AudioBlock<FloatType> block) {
        if (toUpdate.exchange(false)) {
            update();
        }
        if (isON.load()) {
            gain.process(juce::dsp::ProcessContextReplacing<FloatType>(block));
        }
    }

    template
    class StaticGainCompensation<float>;

    template
    class StaticGainCompensation<double>;
} // zlIIR
