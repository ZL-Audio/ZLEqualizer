// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fixed_audio_buffer.hpp"

namespace zldsp::buffer {
    template<typename FloatType>
    FixedAudioBuffer<FloatType>::FixedAudioBuffer(int subBufferSize) :
            input_buffer(2, 441), output_buffer(2, 441),
            sub_spec{44100, 441, 2},
            main_spec{44100, 441, 2} {
        setSubBufferSize(subBufferSize);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::clear() {
        input_buffer.clear();
        output_buffer.clear();
        subBuffer.clear();
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::setSubBufferSize(int subBufferSize) {
        clear();
        // init internal spec
        sub_spec = main_spec;
        sub_spec.maximumBlockSize = static_cast<juce::uint32>(subBufferSize);
        if (sub_spec.maximumBlockSize > 1) {
            latency_in_samples.store(sub_spec.maximumBlockSize);
        } else {
            latency_in_samples.store(0);
        }
        // resize subBuffer, inputBuffer and outputBuffer
        subBuffer.setSize(static_cast<int>(sub_spec.numChannels),
                          static_cast<int>(sub_spec.maximumBlockSize));
        input_buffer.setSize(static_cast<int>(main_spec.numChannels),
                            static_cast<int>(main_spec.maximumBlockSize) + subBufferSize);
        output_buffer.setSize(static_cast<int>(main_spec.numChannels),
                             static_cast<int>(main_spec.maximumBlockSize) + subBufferSize);
        // put latency samples
        if (subBufferSize > 1) {
            juce::AudioBuffer<FloatType> zeroBuffer(input_buffer.getNumChannels(), subBufferSize);
            for (int channel = 0; channel < zeroBuffer.getNumChannels(); ++channel) {
                auto *channelData = zeroBuffer.getWritePointer(channel);
                for (int index = 0; index < subBufferSize; ++index) {
                    channelData[index] = 0;
                }
            }
            input_buffer.push(zeroBuffer, subBufferSize);
        }
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::prepare(juce::dsp::ProcessSpec spec) {
        main_spec = spec;
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::pushBuffer(juce::AudioBuffer<FloatType> &buffer) {
        input_buffer.push(buffer);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::pushBlock(juce::dsp::AudioBlock<FloatType> block) {
        input_buffer.push(block);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::popSubBuffer() {
        input_buffer.pop(subBuffer);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::pushSubBuffer() {
        output_buffer.push(subBuffer);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::popBuffer(juce::AudioBuffer<FloatType> &buffer, bool write) {
        if (write) {
            output_buffer.pop(buffer);
        } else {
            output_buffer.pop(buffer.getNumSamples());
        }
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::popBlock(juce::dsp::AudioBlock<FloatType> block, bool write) {
        if (write) {
            output_buffer.pop(block);
        } else {
            output_buffer.pop(static_cast<int>(block.getNumSamples()));
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
