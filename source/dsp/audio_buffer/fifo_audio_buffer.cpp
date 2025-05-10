// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fifo_audio_buffer.hpp"

namespace zldsp::buffer {
    template<typename FloatType>
    FIFOAudioBuffer<FloatType>::FIFOAudioBuffer(int channels, int buffer_size):
            fifo_(buffer_size) {
        buffer.setSize(channels, buffer_size);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::clear() {
        fifo_.reset();
        buffer.clear();
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::setSize(int channels, int buffer_size) {
        juce::ignoreUnused(channels);
        clear();
        fifo_.setTotalSize(buffer_size + 1);
        buffer.setSize(channels, buffer_size + 1);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::push(const FloatType **samples, int num_samples) {
        jassert (fifo_.getFreeSpace() >= num_samples);
        int start1, size1, start2, size2;
        fifo_.prepareToWrite(num_samples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom(channel, start1, samples[channel], size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom(channel, start2, samples[channel] + size1, size2);
        fifo_.finishedWrite(size1 + size2);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::push(const juce::AudioBuffer<FloatType> &samples,
                                          int num_samples) {
        const int addSamples = num_samples < 0 ? samples.getNumSamples() : num_samples;
        jassert (fifo_.getFreeSpace() >= addSamples);

        int start1, size1, start2, size2;
        fifo_.prepareToWrite(addSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom(channel, start1, samples.getReadPointer(channel, 0),
                                size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom(channel, start2, samples.getReadPointer(channel, size1),
                                size2);
        fifo_.finishedWrite(size1 + size2);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::push(juce::dsp::AudioBlock<FloatType> block, int num_samples) {
        const int addSamples = num_samples < 0 ? static_cast<int>(block.getNumSamples()) : num_samples;
        jassert (fifo_.getFreeSpace() >= addSamples);

        int start1, size1, start2, size2;
        fifo_.prepareToWrite(addSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom(channel, start1, block.getChannelPointer(static_cast<size_t>(channel)),
                                size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom(channel, start2, block.getChannelPointer(static_cast<size_t>(channel)) + size1,
                                size2);
        fifo_.finishedWrite(size1 + size2);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::pop(int num_samples) {
        jassert (fifo_.getNumReady() >= num_samples);
        int start1, size1, start2, size2;
        fifo_.prepareToRead(num_samples, start1, size1, start2, size2);
        fifo_.finishedRead(size1 + size2);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::pop(FloatType **samples, int num_samples) {
        jassert (fifo_.getNumReady() >= num_samples);
        int start1, size1, start2, size2;
        fifo_.prepareToRead(num_samples, start1, size1, start2, size2);
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
        fifo_.finishedRead(size1 + size2);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::pop(juce::AudioBuffer<FloatType> &samples, int num_samples) {
        const int readSamples = num_samples > 0 ? num_samples : samples.getNumSamples();
        jassert (fifo_.getNumReady() >= readSamples);
        int start1, size1, start2, size2;
        fifo_.prepareToRead(readSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                samples.copyFrom(channel, 0, buffer.getReadPointer(channel, start1), size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                samples.copyFrom(channel, size1, buffer.getReadPointer(channel, start2),
                                 size2);
        fifo_.finishedRead(size1 + size2);
    }

    template<typename FloatType>
    void FIFOAudioBuffer<FloatType>::pop(juce::dsp::AudioBlock<FloatType> block, int num_samples) {
        const int readSamples = num_samples > 0 ? num_samples : static_cast<int>(block.getNumSamples());
        jassert (fifo_.getNumReady() >= readSamples);
        int start1, size1, start2, size2;
        fifo_.prepareToRead(readSamples, start1, size1, start2, size2);
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
        fifo_.finishedRead(size1 + size2);
    }

    template
    class FIFOAudioBuffer<float>;

    template
    class FIFOAudioBuffer<double>;
}
