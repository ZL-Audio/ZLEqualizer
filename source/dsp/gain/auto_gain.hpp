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

#include "origin_gain.hpp"

namespace zlGain {
    /**
     * a lock free, thread safe auto-gain class
     * it will hard-clip output signal to 0dB
     * @tparam FloatType
     */
    template<typename FloatType>
    class AutoGain {
    public:
        AutoGain() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            gainDSP.prepare(spec);
            gainDSP.setRampDurationSeconds(1.0);
        }

        void prepareBuffer() {
            if (isON.load() != currentIsON) {
                currentIsON = isON.load();
                gainDSP.setGainLinear(1);
                gainDSP.reset();
                gain.store(FloatType(1));
            }
        }

        void processPre(juce::AudioBuffer<FloatType> &buffer) {
            prepareBuffer();
            if (currentIsON) {
                auto block = juce::dsp::AudioBlock<FloatType>(buffer);
                preRMS = std::max(calculateRMS(block), FloatType(0.000000001));
            }
        }

        void processPre(juce::dsp::AudioBlock<FloatType> block) {
            prepareBuffer();
            if (currentIsON) {
                preRMS = std::max(calculateRMS(block), FloatType(0.000000001));
            }
        }

        template<bool isBypassed = false>
        void processPost(juce::AudioBuffer<FloatType> &buffer) {
            if (currentIsON) {
                auto block = juce::dsp::AudioBlock<FloatType>(buffer);
                processPostBlockInternal<isBypassed>(block);
            }
        }

        template<bool isBypassed = false>
        void processPost(juce::dsp::AudioBlock<FloatType> block) {
            if (currentIsON) {
                processPostBlockInternal<isBypassed>(block);
            }
        }

        void setRampDurationSeconds(double newDurationSeconds) {
            gainDSP.setRampDurationSeconds(newDurationSeconds);
        }

        void enable(const bool f) {
            if (f) {
                isON.store(true);
            } else {
                isON.store(false);
                gain.store(FloatType(1));
            }
        }

        FloatType getGainDecibels() const {
            return juce::Decibels::gainToDecibels(gain.load());
        }

    private:
        std::atomic<bool> isON{false};
        bool currentIsON{false};
        std::atomic<FloatType> gain;
        FloatType preRMS, postRMS;
        OriginGain<FloatType> gainDSP;

        template<bool isBypassed = false>
        void processPostBlockInternal(juce::dsp::AudioBlock<FloatType> block) {
            if (preRMS > FloatType(0.00001)) {
                postRMS = std::max(calculateRMS(block), FloatType(0.000000001));
                gain.store(gainDSP.getCurrentGainLinear());
                gainDSP.setGainLinear(preRMS / postRMS);
            }
            if (!isBypassed) {
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

        FloatType calculateRMS(juce::dsp::AudioBlock<FloatType> block) {
            FloatType _ms = 0;
            for (size_t channel = 0; channel < block.getNumChannels(); channel++) {
                auto data = block.getChannelPointer(channel);
                for (size_t i = 0; i < block.getNumSamples(); i++) {
                    _ms += data[i] * data[i];
                }
            }
            return std::sqrt(_ms / static_cast<FloatType>(block.getNumChannels()));
        }
    };
} // zlGain
