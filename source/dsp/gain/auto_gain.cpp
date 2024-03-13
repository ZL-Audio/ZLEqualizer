// Copyright (C) 2023 - zsliu98
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
        if (isON.load()) {
            if (toReset.load()) {
                gainDSP.setGainLinear(1);
                gainDSP.reset();
                toReset.store(false);
            }
            preRMS = juce::Decibels::gainToDecibels(calculateRMS(block), FloatType(-240));
            isPreProcessed.store(true);
        }
    }

    template<typename FloatType>
    void AutoGain<FloatType>::processPost(juce::AudioBuffer<FloatType> &buffer) {
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        processPost(block);
    }

    template<typename FloatType>
    void AutoGain<FloatType>::processPost(juce::dsp::AudioBlock<FloatType> block) {
        if (isON.load() && isPreProcessed.load()) {
            postRMS = juce::Decibels::gainToDecibels(calculateRMS(block), FloatType(-240));
            gainDSP.setGainDecibels(preRMS - postRMS);
            juce::dsp::ProcessContextReplacing<FloatType> context(block);
            gainDSP.process(context);
        }
        isPreProcessed.store(false);
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
            toReset.store(true);
            isON.store(true);
        } else {
            isON.store(false);
        }
    }

    template
    class AutoGain<float>;

    template
    class AutoGain<double>;
} // zlGain