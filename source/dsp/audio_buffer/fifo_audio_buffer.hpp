// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zldsp::buffer {
    template<typename FloatType>
    class FIFOAudioBuffer {
    public:
        explicit FIFOAudioBuffer(int channels = 2, int buffer_size = 441);

        void clear();

        void setSize(int channels, int buffer_size);

        void push(const FloatType **samples, int num_samples);

        void push(const juce::AudioBuffer<FloatType> &samples, int num_samples = -1);

        void push(juce::dsp::AudioBlock<FloatType> block, int num_samples = -1);

        void pop(int num_samples);

        void pop(FloatType **samples, int num_samples);

        void pop(juce::AudioBuffer<FloatType> &samples, int num_samples = -1);

        void pop(juce::dsp::AudioBlock<FloatType> block, int num_samples = -1);

        inline auto getNumChannels() const { return buffer.getNumChannels(); }

        inline auto getNumSamples() const { return fifo_.getTotalSize() - 1; }

        inline auto getNumReady() const { return fifo_.getNumReady(); }

        inline auto getFreeSpace() const { return fifo_.getFreeSpace(); }

        inline auto isFull() const { return fifo_.getFreeSpace() == 0; }

    private:
        juce::AbstractFifo fifo_;

        /*< The actual audio buffer */
        juce::AudioBuffer<FloatType> buffer;
    };
}
