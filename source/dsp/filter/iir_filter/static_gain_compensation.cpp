// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "static_gain_compensation.hpp"

namespace zlFilter {
    template<typename FloatType>
    StaticGainCompensation<FloatType>::StaticGainCompensation(Empty<FloatType> &filter)
        : target(filter) {
        gainDSP.setGainDecibels(0);
    }

    template<typename FloatType>
    void StaticGainCompensation<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        gainDSP.prepare(spec);
    }

    template<typename FloatType>
    void StaticGainCompensation<FloatType>::update() {
        if (!isON.load()) { return; }
        switch (target.getFilterType()) {
            case peak: {
                const auto portion = juce::jmax(std::abs(target.getGain()) - FloatType(12), FloatType(0)) / FloatType(18);
                const auto f = target.getFreq();
                const auto g = juce::jlimit(FloatType(-12), FloatType(12), target.getGain());
                const auto q = juce::jlimit(FloatType(0.1), FloatType(5), target.getQ());
                const auto est = (FloatType(1) + portion * FloatType(0.75)) * getPeakEstimation(f, g, q);
                gain.store(est);
                gainDSP.setGainDecibels(est);
                break;
            }
            case lowShelf: {
                const auto portion = juce::jmax(std::abs(target.getGain()) - FloatType(12), FloatType(0)) / FloatType(18);
                const auto f = target.getFreq();
                const auto g = juce::jlimit(FloatType(-12), FloatType(12), target.getGain());
                const auto est = (FloatType(0.88) + portion * FloatType(0.66)) * getLowShelfEstimation(f, g);
                gain.store(est);
                gainDSP.setGainDecibels(est);
                break;
            }
            case highShelf: {
                const auto portion = juce::jmax(std::abs(target.getGain()) - FloatType(12), FloatType(0)) / FloatType(18);
                const auto f = target.getFreq();
                const auto g = juce::jlimit(FloatType(-12), FloatType(12), target.getGain());
                const auto est = (FloatType(0.5) + portion * FloatType(0.375)) * getHighShelfEstimation(f, g);
                gain.store(est);
                gainDSP.setGainDecibels(est);
                break;
            }
            case tiltShelf:
            case bandShelf:
            case lowPass:
            case highPass:
            case notch:
            case bandPass:
            default:
                gainDSP.setGainLinear(FloatType(1));
                gain.store(0);
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
            gainDSP.process(juce::dsp::ProcessContextReplacing<FloatType>(block));
        }
    }

    template
    class StaticGainCompensation<float>;

    template
    class StaticGainCompensation<double>;
} // zlIIR
