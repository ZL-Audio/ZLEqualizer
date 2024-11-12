// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "rms_tracker.hpp"

namespace zlCompressor {
    template<typename FloatType>
    RMSTracker<FloatType>::~RMSTracker() {
        loudnessBuffer.clear();
    }

    template<typename FloatType>
    void RMSTracker<FloatType>::reset() {
        mLoudness = FloatType(0);
        loudnessBuffer.clear();
    }

    template<typename FloatType>
    void RMSTracker<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        sampleRate.store(spec.sampleRate);
        reset();
        setMomentarySeconds(currentSeconds.load());
    }

    template<typename FloatType>
    void RMSTracker<FloatType>::process(const juce::AudioBuffer<FloatType> &buffer) {
        // calculate mean square
        FloatType _ms = 0;
        for (auto channel = 0; channel < buffer.getNumChannels(); channel++) {
            auto data = buffer.getReadPointer(channel);
            for (auto i = 0; i < buffer.getNumSamples(); i++) {
                _ms += data[i] * data[i];
            }
        }

        _ms = _ms / static_cast<FloatType>(buffer.getNumSamples());

        const auto nowCurrentSize = currentSize.load();
        while (loudnessBuffer.size() >= nowCurrentSize) {
            mLoudness = mLoudness - loudnessBuffer.pop_front();
        }

        loudnessBuffer.push_back(_ms);
        mLoudness += _ms;
    }

    template<typename FloatType>
    void RMSTracker<FloatType>::setMomentarySeconds(FloatType x) {
        currentSeconds.store(x);
        setMomentarySize(static_cast<size_t>(x * static_cast<FloatType>(sampleRate.load())));
    }

    template<typename FloatType>
    void RMSTracker<FloatType>::setMomentarySize(size_t mSize) {
        mSize = std::max(static_cast<size_t>(1), mSize);
        currentSize.store(mSize);
    }

    template<typename FloatType>
    void RMSTracker<FloatType>::setMaximumMomentarySize(size_t mSize) {
        mSize = std::max(static_cast<size_t>(1), mSize);
        loudnessBuffer.set_capacity(mSize);
    }

    template<typename FloatType>
    FloatType RMSTracker<FloatType>::getMomentaryLoudness() {
        FloatType meanSquare = mLoudness / static_cast<FloatType>(currentSize.load());
        return juce::Decibels::gainToDecibels(meanSquare, minusInfinityDB * 2) * static_cast<FloatType>(0.5);
    }

    template
    class RMSTracker<float>;

    template
    class RMSTracker<double>;
} // zldetector
