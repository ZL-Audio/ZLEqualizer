// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_dsp/juce_dsp.h>

#include "../../container/container.hpp"

namespace zlCompressor {
    /**
     * a tracker that tracks the momentary RMS loudness of the audio signal
     * @tparam FloatType
     */
    template<typename FloatType, bool isPeakMix = false>
    class RMSTracker {
    public:
        inline static FloatType minusInfinityDB = FloatType(-240);

        RMSTracker() = default;

        /**
         * call before prepare to play
         * set the maximum time length of the tracker
         * @param second the maximum time length of the tracker
         */
        void setMaximumMomentarySeconds(const FloatType second) {
            maximumTimeLength = second;
        }

        /**
         * call before processing starts
         * @param sr sampleRate
         */
        void prepare(const double sr) {
            sampleRate.store(sr);
            setMaximumMomentarySize(
                static_cast<size_t>(static_cast<double>(maximumTimeLength) * sr));
            reset();
            setMomentarySeconds(timeLength.load());
        }

        /**
         * update values before processing a buffer
         */
        void prepareBuffer() {
            if (toUpdate.exchange(false)) {
                currentBufferSize = bufferSize.load();
                while (loudnessBuffer.size() > currentBufferSize) {
                    mLoudness -= loudnessBuffer.pop_front();
                }
                if (isPeakMix) {
                    currentPeakMix = peakMix.load();
                    currentPeakMixC = FloatType(1) - currentPeakMix;
                }
            }
        }

        void processSample(const FloatType x) {
            const FloatType square = isPeakMix
                                         ? std::abs(x) * (currentPeakMix + std::abs(x) * currentPeakMixC)
                                         : x * x;
            if (loudnessBuffer.size() == currentBufferSize) {
                mLoudness -= loudnessBuffer.pop_front();
            }
            loudnessBuffer.push_back(square);
            mLoudness += square;
        }

        void processBufferRMS(juce::AudioBuffer<FloatType> &buffer) {
            FloatType _ms = 0;
            for (auto channel = 0; channel < buffer.getNumChannels(); channel++) {
                auto data = buffer.getReadPointer(channel);
                for (auto i = 0; i < buffer.getNumSamples(); i++) {
                    _ms += data[i] * data[i];
                }
            }
            _ms = _ms / static_cast<FloatType>(buffer.getNumSamples());

            if (loudnessBuffer.size() == currentBufferSize) {
                mLoudness -= loudnessBuffer.pop_front();
            }
            loudnessBuffer.push_back(_ms);
            mLoudness += _ms;
        }

        /**
         * thread-safe, lock-free
         * set the time length of the tracker
         * @param second the time length of the tracker
         */
        void setMomentarySeconds(const FloatType second) {
            timeLength.store(second);
            setMomentarySize(static_cast<size_t>(static_cast<double>(second) * sampleRate.load()));
            toUpdate.store(true);
        }

        /**
         * thread-safe, lock-free
         * get the time length of the tracker
         */
        inline size_t getMomentarySize() const {
            return bufferSize.load();
        }

        /**
         * thread-safe, lock-free
         * set the peak-mix portion
         * @param x the peak-mix portion
         */
        void setPeakMix(const FloatType x) {
            peakMix.store(x);
            toUpdate.store(true);
        }

        FloatType getMomentaryLoudness() {
            FloatType meanSquare = mLoudness / static_cast<FloatType>(currentBufferSize);
            return juce::Decibels::gainToDecibels(meanSquare, minusInfinityDB) * FloatType(0.5);
        }

    private:
        FloatType mLoudness{0};
        zlContainer::CircularBuffer<FloatType> loudnessBuffer{1};

        std::atomic<double> sampleRate{48000.0};
        std::atomic<FloatType> timeLength{0};
        FloatType maximumTimeLength{0};
        size_t currentBufferSize{1};
        std::atomic<size_t> bufferSize{1};
        FloatType currentPeakMix{0}, currentPeakMixC{1};
        std::atomic<FloatType> peakMix{0};
        std::atomic<bool> toUpdate{true};

        void reset() {
            mLoudness = FloatType(0);
            loudnessBuffer.clear();
        }

        void setMomentarySize(size_t mSize) {
            mSize = std::max(static_cast<size_t>(1), mSize);
            bufferSize.store(mSize);
        }

        void setMaximumMomentarySize(size_t mSize) {
            mSize = std::max(static_cast<size_t>(1), mSize);
            loudnessBuffer.set_capacity(mSize);
        }
    };
}
