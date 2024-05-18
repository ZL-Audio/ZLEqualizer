// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLECOMP_FIXEDAUDIOBUFFER_H
#define ZLECOMP_FIXEDAUDIOBUFFER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "fifo_audio_buffer.hpp"

namespace zlAudioBuffer {
    template<typename FloatType>
    class FixedAudioBuffer {
    public:
        juce::AudioBuffer<FloatType> subBuffer;

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
            return inputBuffer.getNumReady() >= subBuffer.getNumSamples();
        }

        inline auto getMainSpec() { return mainSpec; }

        inline auto getSubSpec() { return subSpec; }

        inline juce::uint32 getLatencySamples() {
            if (subSpec.maximumBlockSize > 1) {
                return subSpec.maximumBlockSize;
            } else {
                return juce::uint32(0);
            }
        }

    private:
        FIFOAudioBuffer<FloatType> inputBuffer, outputBuffer;
        juce::dsp::ProcessSpec subSpec, mainSpec;
    };
}

#endif //ZLECOMP_FIXEDAUDIOBUFFER_H
