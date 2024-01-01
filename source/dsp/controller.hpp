// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_CONTROLLER_HPP
#define ZLEQUALIZER_CONTROLLER_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "dsp_definitions.hpp"
#include "audio_buffer/audio_buffer.hpp"
#include "dynamic_filter/dynamic_filter.hpp"
#include "splitter/splitter.hpp"

namespace zlDSP {
    template<typename FloatType>
    class Controller : public juce::AsyncUpdater {
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

        void handleAsyncUpdate() override;

//        inline bool getInitCompleted() { return initCompleted.load(); }

    private:
        juce::AudioProcessor &processorRef;
        std::array<zlDynamicFilter::IIRFilter<FloatType>, bandNUM> filters;
        juce::ReadWriteLock paraUpdateLock;

        std::array<std::atomic<lrTypes>, bandNUM> filterLRs;
        zlSplitter::LRSplitter<FloatType> lrMainSplitter, lrSideSplitter;
        zlSplitter::MSSplitter<FloatType> msMainSplitter, msSideSplitter;
        std::atomic<bool> useLR, useMS;

        std::atomic<bool> useDynamic;
        std::atomic<bool> sideChain;

        static inline double subBufferLength = 0.001;
        zlAudioBuffer::FixedAudioBuffer<FloatType> subBuffer;
        std::atomic<int> latencyInSamples;

        std::array<FloatType, zlIIR::frequencies.size()> dBs{};
        juce::ReadWriteLock magLock;

//        std::atomic<bool> initCompleted = false;

//        juce::FileLogger logger{juce::File("/Volumes/Ramdisk/log.txt"), "Filters Attach Log"};
    };
}

#endif //ZLEQUALIZER_CONTROLLER_HPP
