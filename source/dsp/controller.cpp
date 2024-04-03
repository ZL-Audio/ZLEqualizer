// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "controller.hpp"

namespace zlDSP {
    template<typename FloatType>
    Controller<FloatType>::Controller(juce::AudioProcessor &processor)
        : processorRef(processor) {
    }

    template<typename FloatType>
    void Controller<FloatType>::reset() {
        for (auto &f: filters) {
            f.reset();
        }
        soloFilter.reset();
    }

    template<typename FloatType>
    void Controller<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        delay.prepare({spec.sampleRate, spec.maximumBlockSize, 2});
        delay.setMaximumDelayInSamples(static_cast<int>(0.02 * spec.sampleRate) + 1);
        subBuffer.prepare({spec.sampleRate, spec.maximumBlockSize, 4});
        subBuffer.setSubBufferSize(static_cast<int>(subBufferLength * spec.sampleRate));
        triggerAsyncUpdate();
        juce::dsp::ProcessSpec subSpec{spec.sampleRate, subBuffer.getSubSpec().maximumBlockSize, 2};
        for (auto &f: filters) {
            f.prepare(subSpec);
        }
        soloFilter.setFilterType(zlIIR::FilterType::bandPass, false);
        soloFilter.prepare(subSpec);
        lrMainSplitter.prepare(subSpec);
        lrSideSplitter.prepare(subSpec);
        msMainSplitter.prepare(subSpec);
        msSideSplitter.prepare(subSpec);
        outputGain.prepare(subSpec);
        autoGain.prepare(subSpec);
        fftAnalyzezr.prepare(subSpec);
        conflictAnalyzer.prepare(subSpec);
        for (auto &t: {&tracker, &lTracker, &rTracker, &mTracker, &sTracker}) {
            t->prepare(subSpec);
        }
        const auto numRMS = static_cast<size_t>(
            zlDSP::dynRMS::range.end / 1000.f * static_cast<float>(spec.sampleRate));
        for (auto &f: filters) {
            f.getCompressor().getTracker().setMaximumMomentarySize(numRMS);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        juce::AudioBuffer<FloatType> mainBuffer{buffer.getArrayOfWritePointers() + 0, 2, buffer.getNumSamples()};
        juce::AudioBuffer<FloatType> sideBuffer{buffer.getArrayOfWritePointers() + 2, 2, buffer.getNumSamples()};
        // if no side chain, copy the main buffer into the side buffer
        if (!sideChain.load()) {
            sideBuffer.makeCopyOf(mainBuffer, true);
        }
        // process lookahead
        delay.process(mainBuffer);

        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        // const juce::ScopedReadLock scopedLock(paraUpdateLock);
        // ---------------- start sub buffer
        subBuffer.pushBlock(block);
        while (subBuffer.isSubReady()) {
            subBuffer.popSubBuffer();
            auto subMainBuffer = juce::AudioBuffer<FloatType>(subBuffer.subBuffer.getArrayOfWritePointers() + 0,
                                                              2, subBuffer.subBuffer.getNumSamples());
            auto subSideBuffer = juce::AudioBuffer<FloatType>(subBuffer.subBuffer.getArrayOfWritePointers() + 2,
                                                              2, subBuffer.subBuffer.getNumSamples());
            fftAnalyzezr.pushPreFFTBuffer(subMainBuffer);
            fftAnalyzezr.pushSideFFTBuffer(subSideBuffer);
            conflictAnalyzer.pushRefBuffer(subSideBuffer);
            if (isEffectON.load()) {
                if (useSolo.load()) {
                    processSolo();
                } else {
                    processDynamic();
                }
            }
            fftAnalyzezr.pushPostFFTBuffer(subMainBuffer);
            fftAnalyzezr.process();
            conflictAnalyzer.pushMainBuffer(subMainBuffer);
            conflictAnalyzer.process();
            subBuffer.pushSubBuffer();
        }
        subBuffer.popBlock(block);
        // ---------------- end sub buffer
    }

    template<typename FloatType>
    void Controller<FloatType>::processSolo() {
        for (size_t i = 0; i < bandNUM; ++i) {
            filters[i].getBaseFilter().updateParas();
            filters[i].getMainFilter().updateParas();
            filters[i].getTargetFilter().updateParas();
            filters[i].getSideFilter().updateParas();
        }
        // create main sub buffer and side sub buffer
        auto subMainBuffer = juce::AudioBuffer<FloatType>(subBuffer.subBuffer.getArrayOfWritePointers() + 0,
                                                          2, subBuffer.subBuffer.getNumSamples());
        auto subSideBuffer = juce::AudioBuffer<FloatType>(subBuffer.subBuffer.getArrayOfWritePointers() + 2,
                                                          2, subBuffer.subBuffer.getNumSamples());
        if (soloSide.load()) {
            subMainBuffer.makeCopyOf(subSideBuffer, true);
        }
        switch (filterLRs[soloIdx.load()].load()) {
            case lrType::stereo: {
                soloFilter.process(subMainBuffer);
                break;
            }
            case lrType::left: {
                lrMainSplitter.split(subMainBuffer);
                soloFilter.process(lrMainSplitter.getLBuffer());
                lrMainSplitter.getRBuffer().applyGain(0);
                lrMainSplitter.combine(subMainBuffer);
                break;
            }
            case lrType::right: {
                lrMainSplitter.split(subMainBuffer);
                soloFilter.process(lrMainSplitter.getRBuffer());
                lrMainSplitter.getLBuffer().applyGain(0);
                lrMainSplitter.combine(subMainBuffer);
                break;
            }
            case lrType::mid: {
                msMainSplitter.split(subMainBuffer);
                soloFilter.process(msMainSplitter.getMBuffer());
                msMainSplitter.getSBuffer().applyGain(0);
                msMainSplitter.combine(subMainBuffer);
                break;
            }
            case lrType::side: {
                msMainSplitter.split(subMainBuffer);
                soloFilter.process(msMainSplitter.getSBuffer());
                msMainSplitter.getMBuffer().applyGain(0);
                msMainSplitter.combine(subMainBuffer);
                break;
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::processDynamic() {
        // create main sub buffer and side sub buffer
        auto subMainBuffer = juce::AudioBuffer<FloatType>(subBuffer.subBuffer.getArrayOfWritePointers() + 0,
                                                          2, subBuffer.subBuffer.getNumSamples());
        auto subSideBuffer = juce::AudioBuffer<FloatType>(subBuffer.subBuffer.getArrayOfWritePointers() + 2,
                                                          2, subBuffer.subBuffer.getNumSamples());
        autoGain.processPre(subMainBuffer);
        // stereo filters process
        FloatType baseLine = 0;
        if (useTrackers[0].load()) {
            tracker.process(subSideBuffer);
            baseLine = tracker.getMomentaryLoudness();
            if (baseLine <= tracker.minusInfinityDB + 1) {
                baseLine = tracker.minusInfinityDB * FloatType(0.5);
            }
        }
        for (size_t i = 0; i < bandNUM; ++i) {
            if (dynRelatives[i].load()) {
                filters[i].getCompressor().setBaseLine(baseLine);
            } else {
                filters[i].getCompressor().setBaseLine(0);
            }
            if (filterLRs[i].load() == lrType::stereo) {
                filters[i].process(subMainBuffer, subSideBuffer);
            }
        }
        // LR filters process
        if (useLR.load()) {
            FloatType lBaseLine = 0, rBaseLine = 0;
            lrMainSplitter.split(subMainBuffer);
            lrSideSplitter.split(subSideBuffer);
            if (useTrackers[1].load()) {
                lTracker.process(lrSideSplitter.getLBuffer());
                lBaseLine = lTracker.getMomentaryLoudness();
                if (lBaseLine <= lTracker.minusInfinityDB + 1) {
                    lBaseLine = lTracker.minusInfinityDB * FloatType(0.5);
                }
            }
            if (useTrackers[2].load()) {
                rTracker.process(lrSideSplitter.getRBuffer());
                rBaseLine = rTracker.getMomentaryLoudness();
                if (rBaseLine <= rTracker.minusInfinityDB + 1) {
                    rBaseLine = rTracker.minusInfinityDB * FloatType(0.5);
                }
            }
            for (size_t i = 0; i < bandNUM; ++i) {
                if (filterLRs[i].load() == lrType::left) {
                    if (dynRelatives[i].load()) {
                        filters[i].getCompressor().setBaseLine(lBaseLine);
                    } else {
                        filters[i].getCompressor().setBaseLine(0);
                    }
                    filters[i].process(lrMainSplitter.getLBuffer(), lrSideSplitter.getLBuffer());
                } else if (filterLRs[i].load() == lrType::right) {
                    if (dynRelatives[i].load()) {
                        filters[i].getCompressor().setBaseLine(rBaseLine);
                    } else {
                        filters[i].getCompressor().setBaseLine(0);
                    }
                    filters[i].process(lrMainSplitter.getRBuffer(), lrSideSplitter.getRBuffer());
                }
            }
            lrMainSplitter.combine(subMainBuffer);
        }
        // MS filters process
        if (useMS.load()) {
            FloatType mBaseLine = 0, sBaseLine = 0;
            msMainSplitter.split(subMainBuffer);
            msSideSplitter.split(subSideBuffer);
            if (useTrackers[3].load()) {
                mTracker.process(msSideSplitter.getMBuffer());
                mBaseLine = mTracker.getMomentaryLoudness();
                if (mBaseLine <= mTracker.minusInfinityDB + 1) {
                    mBaseLine = mTracker.minusInfinityDB * FloatType(0.5);
                }
            }
            if (useTrackers[4].load()) {
                sTracker.process(msSideSplitter.getSBuffer());
                sBaseLine = sTracker.getMomentaryLoudness();
                if (sBaseLine <= sTracker.minusInfinityDB + 1) {
                    sBaseLine = sTracker.minusInfinityDB * FloatType(0.5);
                }
            }
            for (size_t i = 0; i < bandNUM; ++i) {
                if (filterLRs[i].load() == lrType::mid) {
                    if (dynRelatives[i].load()) {
                        filters[i].getCompressor().setBaseLine(mBaseLine);
                    } else {
                        filters[i].getCompressor().setBaseLine(0);
                    }
                    filters[i].process(msMainSplitter.getMBuffer(), msSideSplitter.getMBuffer());
                } else if (filterLRs[i].load() == lrType::side) {
                    if (dynRelatives[i].load()) {
                        filters[i].getCompressor().setBaseLine(sBaseLine);
                    } else {
                        filters[i].getCompressor().setBaseLine(0);
                    }
                    filters[i].process(msMainSplitter.getSBuffer(), msSideSplitter.getSBuffer());
                }
            }
            msMainSplitter.combine(subMainBuffer);
        }
        for (size_t i = 0; i < bandNUM; ++i) {
            if (filters[i].getDynamicON() && isHistON[i].load()) {
                auto &compressor = filters[i].getCompressor();
                const auto diff = compressor.getBaseLine() - compressor.getTracker().getMomentaryLoudness();
                const auto histIdx = juce::jlimit(0, 80, juce::roundToInt(diff));
                histograms[i].push(static_cast<size_t>(histIdx));
            }
        }
        autoGain.processPost(subMainBuffer);
        outputGain.process(subMainBuffer);
    }

    template<typename FloatType>
    void Controller<FloatType>::setFilterLRs(const lrType::lrTypes x, const size_t idx) {
        // const juce::ScopedWriteLock scopedLock(paraUpdateLock);
        filterLRs[idx].store(x);
        // update useLR and useMS
        useLR.store(false);
        for (auto &lr: filterLRs) {
            if (lr.load() == lrType::left || lr.load() == lrType::right) {
                useLR.store(true);
                break;
            }
        }
        useMS.store(false);
        for (auto &lr: filterLRs) {
            if (lr.load() == lrType::mid || lr.load() == lrType::side) {
                useMS.store(true);
                break;
            }
        }
        updateTrackersON();
    }

    template<typename FloatType>
    void Controller<FloatType>::setDynamicON(const bool x, size_t idx) {
        // const juce::ScopedWriteLock scopedLock(paraUpdateLock);
        filters[idx].setDynamicON(x);
        filters[idx].getMainFilter().setGain(filters[idx].getBaseFilter().getGain(), false);
        filters[idx].getMainFilter().setQ(filters[idx].getBaseFilter().getQ(), true);
    }

    template<typename FloatType>
    void Controller<FloatType>::updateDBs(const lrType::lrTypes lr) {
        dBs.fill(FloatType(0));
        for (size_t i = 0; i < bandNUM; i++) {
            if (filterLRs[i].load() == lr && !filters[i].getBypass()) {
                filters[i].getMainFilter().updateDBs();
                std::transform(dBs.begin(), dBs.end(),
                               filters[i].getMainFilter().getDBs().begin(),
                               dBs.begin(), std::plus<FloatType>());
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::handleAsyncUpdate() {
        const auto latency = static_cast<int>(subBuffer.getLatencySamples()) + static_cast<int>(delay.getDelaySamples());
        processorRef.setLatencySamples(latency);
    }

    template<typename FloatType>
    std::tuple<FloatType, FloatType> Controller<FloatType>::getSoloFilterParas(zlIIR::Filter<FloatType> &baseFilter) {
        switch (baseFilter.getFilterType()) {
            case zlIIR::FilterType::highPass:
            case zlIIR::FilterType::lowShelf: {
                auto soloFreq = static_cast<FloatType>(std::sqrt(1) * std::sqrt(baseFilter.getFreq()));
                auto scale = soloFreq;
                soloFreq = static_cast<FloatType>(std::min(std::max(soloFreq, FloatType(10)), FloatType(20000)));
                auto bw = std::max(std::log2(scale) * 2, FloatType(0.01));
                auto soloQ = 1 / (2 * std::sinh(std::log(FloatType(2)) / 2 * bw));
                soloQ = std::min(std::max(soloQ, FloatType(0.025)), FloatType(25));
                return {soloFreq, soloQ};
            }
            case zlIIR::FilterType::lowPass:
            case zlIIR::FilterType::highShelf: {
                auto soloFreq = static_cast<FloatType>(std::sqrt(subBuffer.getMainSpec().sampleRate / 2) * std::sqrt(
                                                           baseFilter.getFreq()));
                auto scale = soloFreq / baseFilter.getFreq();
                soloFreq = static_cast<FloatType>(std::min(std::max(soloFreq, FloatType(10)), FloatType(20000)));
                auto bw = std::max(std::log2(scale) * 2, FloatType(0.01));
                auto soloQ = 1 / (2 * std::sinh(std::log(FloatType(2)) / 2 * bw));
                soloQ = std::min(std::max(soloQ, FloatType(0.025)), FloatType(25));
                return {soloFreq, soloQ};
            }
            case zlIIR::FilterType::tiltShelf: {
                return {baseFilter.getFreq(), FloatType(0.025)};
            }
            case zlIIR::FilterType::peak:
            case zlIIR::FilterType::notch:
            case zlIIR::FilterType::bandPass:
            case zlIIR::FilterType::bandShelf:
            default: {
                return {baseFilter.getFreq(), baseFilter.getQ()};
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::setSolo(const size_t idx, const bool isSide) {
        FloatType freq, q;
        if (!isSide) {
            std::tie(freq, q) = getSoloFilterParas(filters[idx].getMainFilter());
        } else {
            std::tie(freq, q) = getSoloFilterParas(filters[idx].getSideFilter());
        }
        // soloFilter.setOrder(2, false);
        soloFilter.setFreq(freq, false);
        soloFilter.setQ(q, true);

        soloIdx.store(idx);
        soloSide.store(isSide);

        useSolo.store(true);
    }

    template<typename FloatType>
    void Controller<FloatType>::setRelative(const size_t idx, const bool isRelative) {
        dynRelatives[idx].store(isRelative);
        updateTrackersON();
    }

    template<typename FloatType>
    void Controller<FloatType>::updateTrackersON() {
        for (auto &f: useTrackers) {
            f.store(false);
        }
        for (size_t i = 0; i < filterLRs.size(); ++i) {
            if (dynRelatives[i].load()) {
                const auto idx = static_cast<size_t>(filterLRs[i].load());
                useTrackers[idx].store(true);
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::setLearningHist(const size_t idx, const bool isLearning) {
        if (isLearning) {
            histograms[idx].reset();
        }
        isHistON[idx].store(isLearning);
    }

    template<typename FloatType>
    void Controller<FloatType>::setLookAhead(const float x) {
        delay.setDelaySeconds(static_cast<FloatType>(x / 1000));
        triggerAsyncUpdate();
    }

    template<typename FloatType>
    void Controller<FloatType>::setRMS(const float x) {
        const auto numRMS = static_cast<size_t>(
            x / 1000.f * static_cast<float>(subBuffer.getMainSpec().sampleRate));
        for (auto &f: filters) {
            f.getCompressor().getTracker().setMomentarySize(numRMS);
        }
    }

    template
    class Controller<float>;

    template
    class Controller<double>;
}
