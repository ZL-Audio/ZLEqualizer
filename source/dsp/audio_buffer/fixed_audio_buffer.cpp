// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fixed_audio_buffer.hpp"

namespace zlAudioBuffer {
    template<typename FloatType>
    FixedAudioBuffer<FloatType>::FixedAudioBuffer(int subBufferSize) :
            inputBuffer(2, 441), outputBuffer(2, 441),
            subSpec{44100, 441, 2},
            mainSpec{44100, 441, 2} {
        setSubBufferSize(subBufferSize);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::clear() {
        inputBuffer.clear();
        outputBuffer.clear();
        subBuffer.clear();
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::setSubBufferSize(int subBufferSize) {
        clear();
        // init internal spec
        subSpec = mainSpec;
        subSpec.maximumBlockSize = static_cast<juce::uint32>(subBufferSize);
        if (subSpec.maximumBlockSize > 1) {
            latencyInSamples.store(subSpec.maximumBlockSize);
        } else {
            latencyInSamples.store(0);
        }
        // resize subBuffer, inputBuffer and outputBuffer
        subBuffer.setSize(static_cast<int>(subSpec.numChannels),
                          static_cast<int>(subSpec.maximumBlockSize));
        inputBuffer.setSize(static_cast<int>(mainSpec.numChannels),
                            static_cast<int>(mainSpec.maximumBlockSize) + subBufferSize);
        outputBuffer.setSize(static_cast<int>(mainSpec.numChannels),
                             static_cast<int>(mainSpec.maximumBlockSize) + subBufferSize);
        // put latency samples
        if (subBufferSize > 1) {
            juce::AudioBuffer<FloatType> zeroBuffer(inputBuffer.getNumChannels(), subBufferSize);
            for (int channel = 0; channel < zeroBuffer.getNumChannels(); ++channel) {
                auto *channelData = zeroBuffer.getWritePointer(channel);
                for (int index = 0; index < subBufferSize; ++index) {
                    channelData[index] = 0;
                }
            }
            inputBuffer.push(zeroBuffer, subBufferSize);
        }
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::prepare(juce::dsp::ProcessSpec spec) {
        mainSpec = spec;
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::pushBuffer(juce::AudioBuffer<FloatType> &buffer) {
        inputBuffer.push(buffer);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::pushBlock(juce::dsp::AudioBlock<FloatType> block) {
        inputBuffer.push(block);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::popSubBuffer() {
        inputBuffer.pop(subBuffer);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::pushSubBuffer() {
        outputBuffer.push(subBuffer);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::popBuffer(juce::AudioBuffer<FloatType> &buffer, bool write) {
        if (write) {
            outputBuffer.pop(buffer);
        } else {
            outputBuffer.pop(buffer.getNumSamples());
        }
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::popBlock(juce::dsp::AudioBlock<FloatType> block, bool write) {
        if (write) {
            outputBuffer.pop(block);
        } else {
            outputBuffer.pop(static_cast<int>(block.getNumSamples()));
        }
    }

    template<typename FloatType>
    juce::AudioBuffer<FloatType> FixedAudioBuffer<FloatType>::getSubBufferChannels(
            int channelOffset, int numChannels) {
        return juce::AudioBuffer<FloatType>(
                subBuffer.getArrayOfWritePointers() + channelOffset,
                numChannels, subBuffer.getNumSamples());
    }

    template<typename FloatType>
    juce::dsp::AudioBlock<FloatType> FixedAudioBuffer<FloatType>::getSubBlockChannels(int channelOffset,
                                                                                      int numChannels) {
        return juce::dsp::AudioBlock<FloatType>(subBuffer).getSubsetChannelBlock(static_cast<size_t>(channelOffset),
                                                                                 static_cast<size_t>(numChannels));
    }

    template
    class FixedAudioBuffer<float>;

    template
    class FixedAudioBuffer<double>;
}