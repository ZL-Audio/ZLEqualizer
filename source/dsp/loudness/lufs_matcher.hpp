// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "lufs_meter.hpp"

namespace zldsp::loudness {
    template<typename FloatType, size_t MaxChannels = 2, bool UseLowPass = false>
    class LUFSMatcher {
    public:
        LUFSMatcher() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            preLoudnessMeter.prepare(spec);
            postLoudnessMeter.prepare(spec);
            sampleRate = spec.sampleRate;
            reset();
        }

        void reset() {
            preLoudnessMeter.reset();
            postLoudnessMeter.reset();
            loudnessDiff.store(FloatType(0));
            currentCount = 0.0;
        }

        void process(juce::AudioBuffer<FloatType> &pre, juce::AudioBuffer<FloatType> &post) {
            preLoudnessMeter.process(pre);
            postLoudnessMeter.process(post);
            currentCount += static_cast<double>(pre.getNumSamples());
            if (currentCount >= sampleRate) {
                currentCount -= sampleRate;
                const auto preLoudness = preLoudnessMeter.getIntegratedLoudness();
                const auto postLoudness = postLoudnessMeter.getIntegratedLoudness();
                loudnessDiff.store(postLoudness - preLoudness);
            }
        }

        FloatType getDiff() const {
            return loudnessDiff.load();
        }

    private:
        LUFSMeter<FloatType, MaxChannels, UseLowPass> preLoudnessMeter, postLoudnessMeter;
        std::atomic<FloatType> loudnessDiff{FloatType(0)};
        double sampleRate{48000}, currentCount{0};
    };
}
