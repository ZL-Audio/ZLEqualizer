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
#include "fft_analyzer/fft_analyzer.hpp"
#include "histogram/histogram.hpp"
#include "gain/gain.hpp"
#include "delay/delay.hpp"

namespace zlDSP {
    template<typename FloatType>
    class Controller : public juce::AsyncUpdater {
    public:
        explicit Controller(juce::AudioProcessor &processor);

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<FloatType> &buffer);

        void processBypass();

        inline zlDynamicFilter::IIRFilter<FloatType> &getFilter(const size_t idx) { return filters[idx]; }

        inline std::array<zlDynamicFilter::IIRFilter<FloatType>, bandNUM> &getFilters() { return filters; }

        void setFilterLRs(lrType::lrTypes x, size_t idx);

        inline lrType::lrTypes getFilterLRs(const size_t idx) const { return filterLRs[idx].load(); }

        void setDynamicON(bool x, size_t idx);

        inline std::array<FloatType, zlIIR::frequencies.size()> &getDBs() { return dBs; }

        void updateDBs(lrType::lrTypes lr);

        void handleAsyncUpdate() override;

        void setSolo(size_t idx, bool isSide);

        inline bool getSolo() const { return useSolo.load(); }

        inline void clearSolo() {
            useSolo.store(false);
            // soloFilter.setOrder(2);
        }

        inline size_t getSoloIdx() const { return soloIdx.load(); }

        inline bool getSoloIsSide() const { return soloSide.load(); }

        inline zlIIR::Filter<FloatType> &getSoloFilter() { return soloFilter; }

        std::tuple<FloatType, FloatType> getSoloFilterParas(zlIIR::Filter<FloatType> &baseFilter);

        inline void setSideChain(const bool x) { sideChain.store(x); }

        void setRelative(size_t idx, bool isRelative);

        void setLearningHist(size_t idx, bool isLearning);

        bool getLearningHistON(size_t idx) const { return isHistON[idx].load(); }

        zlHistogram::Histogram<FloatType, 80> &getLearningHist(const size_t idx) { return histograms[idx]; }

        void setLookAhead(FloatType x);

        void setRMS(FloatType x);

        void setEffectON(const bool x) { isEffectON.store(x); }

        zlFFT::PrePostFFTAnalyzer<FloatType> &getAnalyzer() { return fftAnalyzezr; }

        zlFFT::ConflictAnalyzer<FloatType> &getConflictAnalyzer() { return conflictAnalyzer; }

        zlGain::Gain<FloatType> &getGainDSP() { return outputGain; }

        zlGain::AutoGain<FloatType> &getAutoGain() { return autoGain; }

    private:
        juce::AudioProcessor &processorRef;
        std::array<zlDynamicFilter::IIRFilter<FloatType>, bandNUM> filters;

        std::array<std::atomic<lrType::lrTypes>, bandNUM> filterLRs;
        zlSplitter::LRSplitter<FloatType> lrMainSplitter, lrSideSplitter;
        zlSplitter::MSSplitter<FloatType> msMainSplitter, msSideSplitter;
        std::atomic<bool> useLR, useMS;

        std::array<std::atomic<bool>, bandNUM> dynRelatives;
        zlCompressor::RMSTracker<FloatType> tracker, lTracker, rTracker, mTracker, sTracker;
        std::array<std::atomic<bool>, 5> useTrackers;

        std::atomic<bool> sideChain;

        zlIIR::Filter<FloatType> soloFilter;
        std::atomic<size_t> soloIdx;
        std::atomic<bool> useSolo = false, soloSide = false;

        std::array<zlHistogram::Histogram<FloatType, 80>, bandNUM> histograms;
        std::array<std::atomic<bool>, bandNUM> isHistON;

        static inline double subBufferLength = 0.001;
        zlAudioBuffer::FixedAudioBuffer<FloatType> subBuffer;

        std::array<FloatType, zlIIR::frequencies.size()> dBs{};

        zlDelay::SampleDelay<FloatType> delay;

        zlGain::Gain<FloatType> outputGain;

        zlGain::AutoGain<FloatType> autoGain;

        std::atomic<bool> isEffectON{true};

        zlFFT::PrePostFFTAnalyzer<FloatType> fftAnalyzezr{};

        zlFFT::ConflictAnalyzer<FloatType> conflictAnalyzer{};

        void processSolo();

        void processDynamic();

        void updateTrackersON();
    };
}

#endif //ZLEQUALIZER_CONTROLLER_HPP
