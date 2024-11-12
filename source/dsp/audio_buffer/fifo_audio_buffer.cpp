// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fifo_audio_buffer.hpp"

namespace zlAudioBuffer {
    template<typename FloatType>
    FIFOAudioBuffer<FloatType>::FIFOAudioBuffer(int channels, int bufferSize):
            fifo(bufferSize) {
        buffer.setSize(channels, bufferSize);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::clear() {
        fifo.reset();
        buffer.clear();
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::setSize(int channels, int bufferSize) {
        juce::ignoreUnused(channels);
        clear();
        fifo.setTotalSize(bufferSize + 1);
        buffer.setSize(channels, bufferSize + 1);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::push(const FloatType **samples, int numSamples) {
        jassert (fifo.getFreeSpace() >= numSamples);
        int start1, size1, start2, size2;
        fifo.prepareToWrite(numSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom(channel, start1, samples[channel], size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom(channel, start2, samples[channel] + size1, size2);
        fifo.finishedWrite(size1 + size2);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::push(const juce::AudioBuffer<FloatType> &samples,
                                          int numSamples) {
        const int addSamples = numSamples < 0 ? samples.getNumSamples() : numSamples;
        jassert (fifo.getFreeSpace() >= addSamples);

        int start1, size1, start2, size2;
        fifo.prepareToWrite(addSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom(channel, start1, samples.getReadPointer(channel, 0),
                                size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom(channel, start2, samples.getReadPointer(channel, size1),
                                size2);
        fifo.finishedWrite(size1 + size2);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::push(juce::dsp::AudioBlock<FloatType> block, int numSamples) {
        const int addSamples = numSamples < 0 ? static_cast<int>(block.getNumSamples()) : numSamples;
        jassert (fifo.getFreeSpace() >= addSamples);

        int start1, size1, start2, size2;
        fifo.prepareToWrite(addSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom(channel, start1, block.getChannelPointer(static_cast<size_t>(channel)),
                                size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom(channel, start2, block.getChannelPointer(static_cast<size_t>(channel)) + size1,
                                size2);
        fifo.finishedWrite(size1 + size2);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::pop(int numSamples) {
        jassert (fifo.getNumReady() >= numSamples);
        int start1, size1, start2, size2;
        fifo.prepareToRead(numSamples, start1, size1, start2, size2);
        fifo.finishedRead(size1 + size2);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::pop(FloatType **samples, int numSamples) {
        jassert (fifo.getNumReady() >= numSamples);
        int start1, size1, start2, size2;
        fifo.prepareToRead(numSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                juce::FloatVectorOperations::copy(samples[channel],
                                                  buffer.getReadPointer(channel, start1),
                                                  size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                juce::FloatVectorOperations::copy(samples[channel] + size1,
                                                  buffer.getReadPointer(channel, start2),
                                                  size2);
        fifo.finishedRead(size1 + size2);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::pop(juce::AudioBuffer<FloatType> &samples, int numSamples) {
        const int readSamples = numSamples > 0 ? numSamples : samples.getNumSamples();
        jassert (fifo.getNumReady() >= readSamples);
        int start1, size1, start2, size2;
        fifo.prepareToRead(readSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                samples.copyFrom(channel, 0, buffer.getReadPointer(channel, start1), size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                samples.copyFrom(channel, size1, buffer.getReadPointer(channel, start2),
                                 size2);
        fifo.finishedRead(size1 + size2);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::pop(juce::dsp::AudioBlock<FloatType> block, int numSamples) {
        const int readSamples = numSamples > 0 ? numSamples : static_cast<int>(block.getNumSamples());
        jassert (fifo.getNumReady() >= readSamples);
        int start1, size1, start2, size2;
        fifo.prepareToRead(readSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                for (int i = 0; i < size1; ++i) {
                    block.setSample(channel, i, buffer.getSample(channel, i + start1));
                }
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                for (int i = 0; i < size2; ++i) {
                    block.setSample(channel, size1 + i, buffer.getSample(channel, i + start2));
                }
        fifo.finishedRead(size1 + size2);
    }

    template
    class FIFOAudioBuffer<float>;

    template
    class FIFOAudioBuffer<double>;
}