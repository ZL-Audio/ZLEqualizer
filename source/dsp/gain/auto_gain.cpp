// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "auto_gain.hpp"

namespace zlGain {
    template<typename FloatType>
    void AutoGain<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        gainDSP.prepare(spec);
        gainDSP.setRampDurationSeconds(1.0);
    }

    template<typename FloatType>
    void AutoGain<FloatType>::processPre(juce::AudioBuffer<FloatType> &buffer) {
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        processPre(block);
    }

    template<typename FloatType>
    void AutoGain<FloatType>::processPre(juce::dsp::AudioBlock<FloatType> block) {
        if (isON.load() != currentIsON) {
            currentIsON = isON.load();
            gainDSP.setGainLinear(1);
            gainDSP.reset();
            gain.store(FloatType(1));
        }
        if (currentIsON) {
            preRMS = std::max(calculateRMS(block), FloatType(0.000000001));
        }
    }

    template<typename FloatType>
    void AutoGain<FloatType>::processPost(juce::AudioBuffer<FloatType> &buffer) {
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        processPost(block);
    }

    template<typename FloatType>
    void AutoGain<FloatType>::processPost(juce::dsp::AudioBlock<FloatType> block) {
        if (currentIsON) {
            if (preRMS > FloatType(0.00001)) {
                postRMS = std::max(calculateRMS(block), FloatType(0.000000001));
                gain.store(gainDSP.getCurrentGainLinear());
                gainDSP.setGainLinear(preRMS / postRMS);
            }
            juce::dsp::ProcessContextReplacing<FloatType> context(block);
            gainDSP.process(context);
            for (size_t channel = 0; channel < block.getNumChannels(); ++channel) {
                juce::FloatVectorOperations::clip(block.getChannelPointer(channel),
                                                  block.getChannelPointer(channel),
                                                  static_cast<FloatType>(-1.0),
                                                  static_cast<FloatType>(1.0),
                                                  static_cast<int>(block.getNumSamples()));
            }
        }
    }

    template<typename FloatType>
    FloatType AutoGain<FloatType>::calculateRMS(juce::dsp::AudioBlock<FloatType> block) {
        FloatType _ms = 0;
        for (size_t channel = 0; channel < block.getNumChannels(); channel++) {
            auto data = block.getChannelPointer(channel);
            for (size_t i = 0; i < block.getNumSamples(); i++) {
                _ms += data[i] * data[i];
            }
        }
        return std::sqrt(_ms / static_cast<FloatType>(block.getNumChannels()));
    }

    template<typename FloatType>
    void AutoGain<FloatType>::setRampDurationSeconds(double newDurationSeconds) {
        gainDSP.setRampDurationSeconds(newDurationSeconds);
    }

    template<typename FloatType>
    void AutoGain<FloatType>::enable(const bool f) {
        if (f) {
            isON.store(true);
        } else {
            isON.store(false);
            gain.store(FloatType(1));
        }
    }

    template
    class AutoGain<float>;

    template
    class AutoGain<double>;
} // zlGain
