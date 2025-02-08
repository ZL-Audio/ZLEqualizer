// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

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
#include "eq_match/eq_match.hpp"

namespace zlDSP {
    template<typename FloatType>
    class Controller final : public juce::AsyncUpdater {
    public:
        static constexpr size_t FilterSize = 16;

        explicit Controller(juce::AudioProcessor &processor, size_t fftOrder = 12);

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<FloatType> &buffer);

        zlFilter::DynamicIIR<FloatType, FilterSize> &getFilter(const size_t idx) { return filters[idx]; }

        zlFilter::Ideal<FloatType, FilterSize> &getMainIdealFilter(const size_t idx) { return mainIdeals[idx]; }

        zlFilter::IIRIdle<FloatType, FilterSize> &getMainIIRFilter(const size_t idx) { return mainIIRs[idx]; }

        std::array<zlFilter::DynamicIIR<FloatType, FilterSize>, bandNUM> &getFilters() { return filters; }

        void setFilterLRs(lrType::lrTypes x, size_t idx);

        lrType::lrTypes getFilterLRs(const size_t idx) const { return filterLRs[idx].load(); }

        void setDynamicON(bool x, size_t idx);

        void handleAsyncUpdate() override;

        void setSolo(size_t idx, bool isSide);

        bool getSolo() const { return useSolo.load(); }

        bool getSoloIsSide() const { return soloSide.load(); }

        void clearSolo(size_t idx, bool isSide);

        inline zlFilter::IIR<FloatType, FilterSize> &getSoloFilter() { return soloFilter; }

        std::tuple<FloatType, FloatType> getSoloFilterParas(zlFilter::FilterType fType, FloatType freq, FloatType q);

        inline void setSideChain(const bool x) { sideChain.store(x); }

        void setRelative(size_t idx, bool isRelative);

        void setSideSwap(size_t idx, bool isSwap);

        void setLearningHistON(size_t idx, bool isLearning);

        bool getLearningHistON(const size_t idx) const { return isHistON[idx].load(); }

        zlHistogram::AtomicHistogram<FloatType, 80> &getLearningHist(const size_t idx) { return atomicHistograms[idx]; }

        void setLookAhead(FloatType x);

        void setRMS(FloatType x);

        void setEffectON(const bool x) { isEffectON.store(x); }

        zlFFT::PrePostFFTAnalyzer<FloatType> &getAnalyzer() { return fftAnalyzer; }

        zlFFT::ConflictAnalyzer<FloatType> &getConflictAnalyzer() { return conflictAnalyzer; }

        zlEqMatch::EqMatchAnalyzer<FloatType> &getMatchAnalyzer() { return matchAnalyzer; }

        zlGain::Gain<FloatType> &getGainDSP() { return outputGain; }

        zlGain::AutoGain<FloatType> &getAutoGain() { return autoGain; }

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
            mFilterStructure.store(x);
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
                    return std::array{
                        zlFilter::DynamicIIR<FloatType, FilterSize>{std::get<Is>(bFilters), std::get<Is>(tFilters)}...
                    };
                }(std::make_index_sequence<std::tuple_size_v<decltype(bFilters)> >());

        std::array<std::atomic<lrType::lrTypes>, bandNUM> filterLRs;
        std::array<lrType::lrTypes, bandNUM> currentFilterLRs{};
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

        std::array<zlFilter::IIRIdle<FloatType, FilterSize>, bandNUM> mainIIRs;
        std::array<zlFilter::Ideal<FloatType, FilterSize>, bandNUM> mainIdeals;

        std::vector<std::complex<FloatType> > prototypeW1, prototypeW2;
        std::array<zlFilter::PrototypeCorrection<FloatType, bandNUM, FilterSize>, 5> prototypeCorrections =
                [&]<size_t... Is>(std::index_sequence<Is...>) {
                    return std::array{
                        zlFilter::PrototypeCorrection<FloatType, bandNUM, FilterSize>{
                            mainIIRs, mainIdeals, std::get<Is>(filterLRIndices), currentIsBypass, prototypeW1,
                            prototypeW2
                        }...
                    };
                }(std::make_index_sequence<std::tuple_size_v<decltype(filterLRIndices)> >());

        std::vector<std::complex<FloatType> > mixedW1, mixedW2;
        std::array<zlFilter::MixedCorrection<FloatType, bandNUM, FilterSize>, 5> mixedCorrections =
                [&]<size_t... Is>(std::index_sequence<Is...>) {
                    return std::array{
                        zlFilter::MixedCorrection<FloatType, bandNUM, FilterSize>{
                            mainIIRs, mainIdeals, std::get<Is>(filterLRIndices), currentIsBypass, mixedW1, mixedW2
                        }...
                    };
                }(std::make_index_sequence<std::tuple_size_v<decltype(filterLRIndices)> >());

        std::vector<std::complex<FloatType> > linearW1;
        std::array<zlFilter::FIR<FloatType, bandNUM, FilterSize>, 5> linearFilters =
                [&]<size_t... Is>(std::index_sequence<Is...>) {
                    return std::array{
                        zlFilter::FIR<FloatType, bandNUM, FilterSize>{
                            mainIdeals, std::get<Is>(filterLRIndices), currentIsBypass, linearW1
                        }...
                    };
                }(std::make_index_sequence<std::tuple_size_v<decltype(filterLRIndices)> >());


        std::atomic<int> latency{0};

        zlSplitter::LRSplitter<FloatType> lrMainSplitter, lrSideSplitter;
        zlSplitter::MSSplitter<FloatType> msMainSplitter, msSideSplitter;

        std::array<std::atomic<bool>, bandNUM> dynRelatives, sideSwaps;
        std::atomic<bool> toUpdateDynRelSide{true};
        std::array<bool, bandNUM> currentDynRelatives{}, currentSideSwaps{};
        std::array<zlCompressor::RMSTracker<FloatType>, 5> trackers;
        std::array<bool, 5> useTrackers{};
        std::array<FloatType, 5> trackerBaselines{};

        std::atomic<bool> sideChain;

        zlFilter::IIR<FloatType, FilterSize> soloFilter;
        std::atomic<size_t> soloIdx;
        std::atomic<bool> toUpdateSolo{false};
        std::atomic<bool> useSolo{false}, soloSide{false};
        bool currentUseSolo{false}, currentSoloSide{false};
        size_t currentSoloIdx{0};

        std::array<zlHistogram::Histogram<FloatType, 80>, bandNUM> histograms;
        std::array<zlHistogram::Histogram<FloatType, 80>, bandNUM> subHistograms;
        std::array<zlHistogram::AtomicHistogram<FloatType, 80>, bandNUM> atomicHistograms;
        std::array<std::atomic<bool>, bandNUM> isHistON{};
        std::array<bool, bandNUM> currentIsHistON{};
        std::atomic<bool> toUpdateHist{true};
        std::array<std::atomic<FloatType>, bandNUM> currentThreshold{};

        static inline double subBufferLength = 0.001;
        zlAudioBuffer::FixedAudioBuffer<FloatType> subBuffer;

        zlDelay::SampleDelay<FloatType> delay;

        zlGain::Gain<FloatType> outputGain;

        zlGain::AutoGain<FloatType> autoGain;

        std::atomic<bool> isEffectON{true};
        bool currentIsEffectON{true};

        zlFFT::PrePostFFTAnalyzer<FloatType> fftAnalyzer;

        zlFFT::ConflictAnalyzer<FloatType> conflictAnalyzer;

        zlEqMatch::EqMatchAnalyzer<FloatType> matchAnalyzer;

        std::atomic<double> sampleRate{48000};

        std::atomic<bool> isZeroLatency{false};

        zlPhase::PhaseFlip<FloatType> phaseFlipper;

        std::atomic<filterStructure::FilterStructure> mFilterStructure{filterStructure::minimum};
        filterStructure::FilterStructure currentFilterStructure{filterStructure::minimum};

        void processSubBuffer(juce::AudioBuffer<FloatType> &subMainBuffer,
                              juce::AudioBuffer<FloatType> &subSideBuffer);

        template<bool isBypassed = false>
        void processSubBufferOnOff(juce::AudioBuffer<FloatType> &subMainBuffer,
                                   juce::AudioBuffer<FloatType> &subSideBuffer);

        void processSolo(juce::AudioBuffer<FloatType> &subMainBuffer,
                         juce::AudioBuffer<FloatType> &subSideBuffer);

        template<bool isBypassed = false>
        void processDynamic(juce::AudioBuffer<FloatType> &subMainBuffer,
                            juce::AudioBuffer<FloatType> &subSideBuffer);

        template<size_t lrIdx>
        void processDynamicLRMSTrackers(juce::AudioBuffer<FloatType> &subSideBuffer);

        template<bool isBypassed = false, size_t lrIdx1, size_t lrIdx2>
        void processDynamicLRMS(juce::AudioBuffer<FloatType> &subMainBuffer,
                                juce::AudioBuffer<FloatType> &subSideBuffer1,
                                juce::AudioBuffer<FloatType> &subSideBuffer2);

        template<bool isBypassed = false>
        void processParallelPost(juce::AudioBuffer<FloatType> &subMainBuffer,
                                 juce::AudioBuffer<FloatType> &subSideBuffer);

        template<bool isBypassed = false>
        void processParallelPostLRMS(size_t lrIdx,
                                     bool shouldParallel,
                                     juce::AudioBuffer<FloatType> &subMainBuffer,
                                     juce::AudioBuffer<FloatType> &subSideBuffer);

        template<bool isBypassed = false>
        void processPrototypeCorrection(juce::AudioBuffer<FloatType> &subMainBuffer);

        template<bool isBypassed = false>
        void processMixedCorrection(juce::AudioBuffer<FloatType> &subMainBuffer);

        template<bool isBypassed = false>
        void processLinear(juce::AudioBuffer<FloatType> &subMainBuffer);

        void updateLRs();

        void updateDynamicONs();

        void updateTrackersON();

        void updateSgcValues();

        void updateSubBuffer();

        void updateFilterStructure();

        void updateCorrections();

        void updateSolo();

        void updateDynRelSide();

        void updateHistograms();
    };
}

#endif //ZLEQUALIZER_CONTROLLER_HPP
