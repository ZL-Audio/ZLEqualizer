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
#include "fifo_audio_buffer.hpp"

namespace zldsp::buffer {
    template<typename FloatType>
    class FixedAudioBuffer {
    public:
        juce::AudioBuffer<FloatType> sub_buffer_;

        explicit FixedAudioBuffer(int subBufferSize = 1);

        void clear();

        void setSubBufferSize(int subBufferSize);

        void prepare(juce::dsp::ProcessSpec spec);

        void pushBuffer(juce::AudioBuffer<FloatType> &buffer);

        void pushBlock(juce::dsp::AudioBlock<FloatType> block);

        void popSubBuffer();

        void pushSubBuffer();

        void popBuffer(juce::AudioBuffer<FloatType> &buffer, bool write = true);

        void popBlock(juce::dsp::AudioBlock<FloatType> block, bool write = true);

        juce::AudioBuffer<FloatType> getSubBufferChannels(int channelOffset, int numChannels);

        juce::dsp::AudioBlock<FloatType> getSubBlockChannels(int channelOffset, int numChannels);

        inline auto isSubReady() {
            return input_buffer_.getNumReady() >= sub_buffer_.getNumSamples();
        }

        inline auto getMainSpec() { return main_spec_; }

        inline auto getSubSpec() { return sub_spec_; }

        inline juce::uint32 getLatencySamples() const {
            return static_cast<juce::uint32>(latency_in_samples_.load());
        }

    private:
        FIFOAudioBuffer<FloatType> input_buffer_, output_buffer_;
        juce::dsp::ProcessSpec sub_spec_, main_spec_;
        std::atomic<juce::uint32> latency_in_samples_{0};
    };
}
