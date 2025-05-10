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
            input_buffer_(2, 441), output_buffer_(2, 441),
            sub_spec_{44100, 441, 2},
            main_spec_{44100, 441, 2} {
        setSubBufferSize(subBufferSize);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::clear() {
        input_buffer_.clear();
        output_buffer_.clear();
        sub_buffer_.clear();
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::setSubBufferSize(int subBufferSize) {
        clear();
        // init internal spec
        sub_spec_ = main_spec_;
        sub_spec_.maximumBlockSize = static_cast<juce::uint32>(subBufferSize);
        if (sub_spec_.maximumBlockSize > 1) {
            latency_in_samples_.store(sub_spec_.maximumBlockSize);
        } else {
            latency_in_samples_.store(0);
        }
        // resize subBuffer, inputBuffer and outputBuffer
        sub_buffer_.setSize(static_cast<int>(sub_spec_.numChannels),
                          static_cast<int>(sub_spec_.maximumBlockSize));
        input_buffer_.setSize(static_cast<int>(main_spec_.numChannels),
                            static_cast<int>(main_spec_.maximumBlockSize) + subBufferSize);
        output_buffer_.setSize(static_cast<int>(main_spec_.numChannels),
                             static_cast<int>(main_spec_.maximumBlockSize) + subBufferSize);
        // put latency samples
        if (subBufferSize > 1) {
            juce::AudioBuffer<FloatType> zeroBuffer(input_buffer_.getNumChannels(), subBufferSize);
            for (int channel = 0; channel < zeroBuffer.getNumChannels(); ++channel) {
                auto *channelData = zeroBuffer.getWritePointer(channel);
                for (int index = 0; index < subBufferSize; ++index) {
                    channelData[index] = 0;
                }
            }
            input_buffer_.push(zeroBuffer, subBufferSize);
        }
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::prepare(juce::dsp::ProcessSpec spec) {
        main_spec_ = spec;
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::pushBuffer(juce::AudioBuffer<FloatType> &buffer) {
        input_buffer_.push(buffer);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::pushBlock(juce::dsp::AudioBlock<FloatType> block) {
        input_buffer_.push(block);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::popSubBuffer() {
        input_buffer_.pop(sub_buffer_);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::pushSubBuffer() {
        output_buffer_.push(sub_buffer_);
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::popBuffer(juce::AudioBuffer<FloatType> &buffer, bool write) {
        if (write) {
            output_buffer_.pop(buffer);
        } else {
            output_buffer_.pop(buffer.getNumSamples());
        }
    }

    template<typename FloatType>
    void FixedAudioBuffer<FloatType>::popBlock(juce::dsp::AudioBlock<FloatType> block, bool write) {
        if (write) {
            output_buffer_.pop(block);
        } else {
            output_buffer_.pop(static_cast<int>(block.getNumSamples()));
        }
    }

    template<typename FloatType>
    juce::AudioBuffer<FloatType> FixedAudioBuffer<FloatType>::getSubBufferChannels(
            int channelOffset, int numChannels) {
        return juce::AudioBuffer<FloatType>(
                sub_buffer_.getArrayOfWritePointers() + channelOffset,
                numChannels, sub_buffer_.getNumSamples());
    }

    template<typename FloatType>
    juce::dsp::AudioBlock<FloatType> FixedAudioBuffer<FloatType>::getSubBlockChannels(int channelOffset,
                                                                                      int numChannels) {
        return juce::dsp::AudioBlock<FloatType>(sub_buffer_).getSubsetChannelBlock(static_cast<size_t>(channelOffset),
                                                                                 static_cast<size_t>(numChannels));
    }

    template
    class FixedAudioBuffer<float>;

    template
    class FixedAudioBuffer<double>;
}
