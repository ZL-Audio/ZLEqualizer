// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_CONTROLLER_H
#define ZLEQUALIZER_CONTROLLER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "dsp_definitions.h"
#include "audio_buffer/audio_buffer.h"
#include "dynamic_filter/dynamic_filter.h"
#include "splitter/splitter.h"

namespace zlDSP {
    template<typename FloatType>
    class Controller {
    public:
        explicit Controller(juce::AudioProcessor &processor);

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<FloatType> &buffer);

        inline zlDynamicFilter::IIRFilter<FloatType> &getFilter(size_t idx) { return filters[idx]; }

        inline std::array<zlDynamicFilter::IIRFilter<FloatType>, bandNUM> &getFilters() { return filters; }

        void setFilterLRs(lrTypes x, size_t idx);

        inline lrTypes getFilterLRs(size_t idx) { return filterLRs[idx].load(); }

        void setDynamicON(bool x, size_t idx);

        void addDBs(std::array<FloatType, zlIIR::frequencies.size()> &x);

        void updateDBs();

    private:
        juce::AudioProcessor &processorRef;
        std::array<zlDynamicFilter::IIRFilter<FloatType>, bandNUM> filters;

        std::array<std::atomic<lrTypes>, bandNUM> filterLRs;
        zlSplitter::LRSplitter<FloatType> lrMainSplitter, lrSideSplitter;
        zlSplitter::MSSplitter<FloatType> msMainSplitter, msSideSplitter;
        std::atomic<bool> useLR, useMS;

        std::atomic<bool> useDynamic;
        std::atomic<bool> sideChain;

        static inline double subBufferLength = 0.001;
        zlAudioBuffer::FixedAudioBuffer<FloatType> subBuffer;

        std::array<FloatType, zlIIR::frequencies.size()> dBs{};
        juce::ReadWriteLock magLock;
    };
}

#endif //ZLEQUALIZER_CONTROLLER_H
