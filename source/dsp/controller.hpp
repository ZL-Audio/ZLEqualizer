// Copyright (C) 2024 - zsliu98
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
#include "filter/filter.hpp"
#include "splitter/splitter.hpp"
#include "fft_analyzer/fft_analyzer.hpp"
#include "histogram/histogram.hpp"
#include "gain/gain.hpp"
#include "delay/delay.hpp"
#include "phase/phase.hpp"
#include "container/container.hpp"

namespace zlDSP {
    template<typename FloatType>
    class Controller final : public juce::AsyncUpdater {
    public:
        static constexpr size_t FilterSize = 16;
        explicit Controller(juce::AudioProcessor &processor);

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<FloatType> &buffer);

        void processBypass();

        inline zlFilter::DynamicIIR<FloatType, FilterSize> &getFilter(const size_t idx) { return filters[idx]; }

        inline zlFilter::Empty<FloatType> &getMainFilter(const size_t idx) { return mainFilters[idx]; }

        inline std::array<zlFilter::DynamicIIR<FloatType, FilterSize>, bandNUM> &getFilters() { return filters; }

        void setFilterLRs(lrType::lrTypes x, size_t idx);

        inline lrType::lrTypes getFilterLRs(const size_t idx) const { return filterLRs[idx].load(); }

        void setDynamicON(bool x, size_t idx);

        void handleAsyncUpdate() override;

        void setSolo(size_t idx, bool isSide);

        inline bool getSolo() const { return useSolo.load(); }

        inline void clearSolo() {
            useSolo.store(false);
            isSoloUpdated.store(true);
        }

        bool exchangeSoloUpdated(const bool x) { return isSoloUpdated.exchange(x); }

        inline size_t getSoloIdx() const { return soloIdx.load(); }

        inline bool getSoloIsSide() const { return soloSide.load(); }

        inline zlFilter::IIR<FloatType, FilterSize> &getSoloFilter() { return soloFilter; }

        std::tuple<FloatType, FloatType> getSoloFilterParas(zlFilter::FilterType fType, FloatType freq, FloatType q);

        inline void setSideChain(const bool x) { sideChain.store(x); }

        void setRelative(size_t idx, bool isRelative);

        void setLearningHist(size_t idx, bool isLearning);

        bool getLearningHistON(size_t idx) const { return isHistON[idx].load(); }

        zlHistogram::Histogram<FloatType, 80> &getLearningHist(const size_t idx) { return histograms[idx]; }

        void setLookAhead(FloatType x);

        void setRMS(FloatType x);

        void setEffectON(const bool x) { isEffectON.store(x); }

        zlFFT::PrePostFFTAnalyzer<FloatType> &getAnalyzer() { return fftAnalyzer; }

        zlFFT::ConflictAnalyzer<FloatType> &getConflictAnalyzer() { return conflictAnalyzer; }

        zlGain::Gain<FloatType> &getGainDSP() { return outputGain; }

        zlGain::AutoGain<FloatType> &getAutoGain() { return autoGain; }

        void setDynLink(const bool x) { dynLink.store(x); }

        bool getDynLink() const { return dynLink.load(); }

        void setZeroLatency(const bool x) {
            isZeroLatency.store(x);
            triggerAsyncUpdate();
        }

        FloatType getGainCompensation() const {
            FloatType currentGain = outputGain.getGainDecibels() + autoGain.getGainDecibels();
            currentGain += compensationGains[0].getGainDecibels() +
                FloatType(0.5) * (compensationGains[1].getGainDecibels() +
                    compensationGains[2].getGainDecibels()) +
                        FloatType(0.95) * compensationGains[3].getGainDecibels() +
                            FloatType(0.05) * compensationGains[4].getGainDecibels();
            return currentGain;
        }

        void setThreshold(const size_t idx, const FloatType x) {
            currentThreshold[idx].store(x);
        }

        FloatType getThreshold(const size_t idx) const {
            return currentThreshold[idx].load();
        }

        zlPhase::PhaseFlip<FloatType> &getPhaseFlipper() { return phaseFlipper; }

        void setFilterStructure(const filterStructure::FilterStructure x) {
            filterStructure.store(x);
        }

        void setIsActive(const size_t idx, const bool flag) {
            filters[idx].setActive(flag);
            isActive[idx].store(flag);
            toUpdateLRs.store(true);
        }

        void updateSgc(const size_t idx) {
            compensations[idx].setToUpdate();
            toUpdateSgc.store(true);
        }

        void setSgcON(const bool x) {
            isSgcON.store(x);
        }

        void setBypass(const size_t idx, const bool x) {
            isBypass[idx].store(x);
            toUpdateBypass.store(true);
        }

        bool getBypass(const size_t idx) const {
            return isBypass[idx].load();
        }

        zlFilter::Empty<FloatType> &getBaseFilter(const size_t idx) {
            return bFilters[idx];
        }

        zlFilter::Empty<FloatType> &getTargetFilter(const size_t idx) {
            return tFilters[idx];
        }

    private:
        juce::AudioProcessor &processorRef;
        std::array<zlFilter::Empty<FloatType>, bandNUM> bFilters, tFilters;

        std::array<zlFilter::DynamicIIR<FloatType, FilterSize>, bandNUM> filters =
            [&]<size_t... Is>(std::index_sequence<Is...>) {
            return std::array{zlFilter::DynamicIIR<FloatType, FilterSize>{std::get<Is>(bFilters), std::get<Is>(tFilters)}...};
        }(std::make_index_sequence<std::tuple_size_v<decltype(bFilters)> >());

        std::array<zlFilter::Empty<FloatType>, bandNUM> mainFilters;
        std::array<std::atomic<lrType::lrTypes>, bandNUM> filterLRs;
        std::array<zlContainer::FixedMaxSizeArray<size_t, bandNUM>, 5> filterLRIndices;
        std::atomic<bool> toUpdateLRs{true};
        bool useLR{false}, useMS{false};

        zlContainer::FixedMaxSizeArray<size_t, bandNUM> dynamicONIndices;
        std::atomic<bool> toUpdateDynamicON{true};

        std::array<zlFilter::StaticGainCompensation<FloatType>, bandNUM> compensations =
            [&]<size_t... Is>(std::index_sequence<Is...>) {
                return std::array{zlFilter::StaticGainCompensation<FloatType>{std::get<Is>(bFilters)}...};
            }(std::make_index_sequence<std::tuple_size_v<decltype(bFilters)> >());

        std::array<zlGain::Gain<FloatType>, 5> compensationGains;
        std::atomic<bool> isSgcON{false}, toUpdateSgc{false};
        bool currentIsSgcON{false};

        std::array<std::atomic<bool>, bandNUM> isActive{};
        std::array<std::atomic<bool>, bandNUM> isBypass{};
        std::array<bool, bandNUM> currentIsBypass{};
        std::atomic<bool> toUpdateBypass;

        zlSplitter::LRSplitter<FloatType> lrMainSplitter, lrSideSplitter;
        zlSplitter::MSSplitter<FloatType> msMainSplitter, msSideSplitter;

        std::array<std::atomic<bool>, bandNUM> dynRelatives;
        std::array<zlCompressor::RMSTracker<FloatType>, 5> trackers;
        std::array<bool, 5> useTrackers{};

        std::atomic<bool> sideChain;

        zlFilter::IIR<FloatType, FilterSize> soloFilter;
        std::atomic<size_t> soloIdx;
        std::atomic<bool> useSolo = false, soloSide = false;

        std::array<zlHistogram::Histogram<FloatType, 80>, bandNUM> histograms;
        std::array<zlHistogram::Histogram<FloatType, 80>, bandNUM> subHistograms;
        std::array<std::atomic<bool>, bandNUM> isHistON{};
        std::array<std::atomic<FloatType>, bandNUM> currentThreshold{};

        static inline double subBufferLength = 0.001;
        zlAudioBuffer::FixedAudioBuffer<FloatType> subBuffer;

        zlDelay::SampleDelay<FloatType> delay;

        zlGain::Gain<FloatType> outputGain;

        zlGain::AutoGain<FloatType> autoGain;

        std::atomic<bool> isEffectON{true};

        zlFFT::PrePostFFTAnalyzer<FloatType> fftAnalyzer{};

        zlFFT::ConflictAnalyzer<FloatType> conflictAnalyzer{};

        std::atomic<bool> dynLink{false};

        std::atomic<double> sampleRate{48000};

        std::atomic<bool> isZeroLatency{false};

        std::atomic<bool> isSoloUpdated{false};

        zlPhase::PhaseFlip<FloatType> phaseFlipper;

        std::atomic<filterStructure::FilterStructure> filterStructure{filterStructure::minimum};
        filterStructure::FilterStructure currentFilterStructure{filterStructure::minimum};

        void processSubBuffer(juce::AudioBuffer<FloatType> &subMainBuffer,
                              juce::AudioBuffer<FloatType> &subSideBuffer);

        void processSolo(juce::AudioBuffer<FloatType> &subMainBuffer,
                         juce::AudioBuffer<FloatType> &subSideBuffer);

        void processDynamic(juce::AudioBuffer<FloatType> &subMainBuffer,
                            juce::AudioBuffer<FloatType> &subSideBuffer);

        void processDynamicLRMS(size_t lrIdx,
                                juce::AudioBuffer<FloatType> &subMainBuffer,
                                juce::AudioBuffer<FloatType> &subSideBuffer);

        void processParallelPostLRMS(size_t lrIdx,
                                     bool shouldParallel,
                                     juce::AudioBuffer<FloatType> &subMainBuffer);

        void processParallelPost(juce::AudioBuffer<FloatType> &subMainBuffer);

        void updateLRs();

        void updateDynamicONs();

        void updateTrackersON();

        void updateSgcValues();

        void updateSubBuffer();

        void updateFilterStructure();
    };
}

#endif //ZLEQUALIZER_CONTROLLER_HPP
