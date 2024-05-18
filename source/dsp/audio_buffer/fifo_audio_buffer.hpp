// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLECOMP_FIFOAUDIOBUFFER_H
#define ZLECOMP_FIFOAUDIOBUFFER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlAudioBuffer {
    template<typename FloatType>
    class FIFOAudioBuffer {
    public:
        explicit FIFOAudioBuffer(int channels=2, int bufferSize=441);

        void clear();

        void setSize(int channels, int bufferSize);

        void push(const FloatType **samples, int numSamples);

        void push(const juce::AudioBuffer<FloatType> &samples, int numSamples = -1);

        void push(juce::dsp::AudioBlock<FloatType> block, int numSamples = -1);

        void pop(int numSamples);

        void pop(FloatType **samples, int numSamples);

        void pop(juce::AudioBuffer<FloatType> &samples, int numSamples = -1);

        void pop(juce::dsp::AudioBlock<FloatType> block, int numSamples = -1);

        inline auto getNumChannels() const { return buffer.getNumChannels(); }

        inline auto getNumSamples() const { return fifo.getTotalSize() - 1; }

        inline auto getNumReady() const { return fifo.getNumReady(); }

        inline auto getFreeSpace() const { return fifo.getFreeSpace(); }

        inline auto isFull() const { return fifo.getFreeSpace() == 0; }

    private:
        juce::AbstractFifo fifo;

        /*< The actual audio buffer */
        juce::AudioBuffer<FloatType> buffer;
    };
}

#endif //ZLECOMP_FIFOAUDIOBUFFER_H
