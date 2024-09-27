// Copyright (C) 2024 - zsliu98
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
        for (size_t i = 0; i < bandNUM; ++i) {
            histograms[i].setDecayRate(FloatType(0.99999));
            subHistograms[i].setDecayRate(FloatType(0.9995));
        }
        soloFilter.setFilterStructure(zlFilter::FilterStructure::svf);
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
        delay.setMaximumDelayInSamples(static_cast<int>(
                                           zlDSP::dynLookahead::range.end / 1000.f * static_cast<float>(spec.
                                               sampleRate)) + 1);
        delay.prepare({spec.sampleRate, spec.maximumBlockSize, 2});

        subBuffer.prepare({spec.sampleRate, spec.maximumBlockSize, 4});
        sampleRate.store(spec.sampleRate);
        updateSubBuffer();
    }

    template<typename FloatType>
    void Controller<FloatType>::updateSubBuffer() {
        subBuffer.setSubBufferSize(static_cast<int>(subBufferLength * sampleRate.load()));

        triggerAsyncUpdate();

        const auto numRMS = static_cast<size_t>(
            zlDSP::dynRMS::range.end / 1000.f * static_cast<float>(sampleRate.load()));
        for (auto &f: filters) {
            f.getCompressor().getTracker().setMaximumMomentarySize(numRMS);
        }

        juce::dsp::ProcessSpec subSpec{sampleRate.load(), subBuffer.getSubSpec().maximumBlockSize, 2};
        for (auto &f: filters) {
            f.prepare(subSpec);
        }

        soloFilter.setFilterType(zlFilter::FilterType::bandPass, false);
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
    }

    template<typename FloatType>
    void Controller<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        if (filterStructure.load() != currentFtilerStructure) {
            currentFtilerStructure = filterStructure.load();
            updateFilterStructure();
        }
        if (toUpdateLRs.exchange(false)) {
            useLR = false;
            useMS = false;
            for (size_t i = 0; i < bandNUM; ++i) {
                currentFilterLRs[i] = filterLRs[i].load();
                useLR = useLR || (currentFilterLRs[i] == lrType::left || currentFilterLRs[i] == lrType::right);
                useMS = useMS || (currentFilterLRs[i] == lrType::mid || currentFilterLRs[i] == lrType::side);
            }
            updateTrackersON();
        }
        juce::AudioBuffer<FloatType> mainBuffer{buffer.getArrayOfWritePointers() + 0, 2, buffer.getNumSamples()};
        juce::AudioBuffer<FloatType> sideBuffer{buffer.getArrayOfWritePointers() + 2, 2, buffer.getNumSamples()};
        // if no side chain, copy the main buffer into the side buffer
        if (!sideChain.load()) {
            sideBuffer.makeCopyOf(mainBuffer, true);
        }
        // process lookahead
        delay.process(mainBuffer);
        if (isZeroLatency.load()) {
            int startSample = 0;
            const int samplePerBuffer = static_cast<int>(subBuffer.getSubSpec().maximumBlockSize);
            while (startSample < buffer.getNumSamples()) {
                const int actualNumSample = std::min(samplePerBuffer, buffer.getNumSamples() - startSample);
                auto subMainBuffer = juce::AudioBuffer<FloatType>(mainBuffer.getArrayOfWritePointers(),
                                                                  2, startSample, actualNumSample);
                auto subSideBuffer = juce::AudioBuffer<FloatType>(sideBuffer.getArrayOfWritePointers(),
                                                                  2, startSample, actualNumSample);
                processSubBuffer(subMainBuffer, subSideBuffer);
                startSample += samplePerBuffer;
            }
        } else {
            auto block = juce::dsp::AudioBlock<FloatType>(buffer);
            // ---------------- start sub buffer
            subBuffer.pushBlock(block);
            while (subBuffer.isSubReady()) {
                subBuffer.popSubBuffer();
                // create main sub buffer and side sub buffer
                auto subMainBuffer = juce::AudioBuffer<FloatType>(subBuffer.subBuffer.getArrayOfWritePointers() + 0,
                                                                  2, subBuffer.subBuffer.getNumSamples());
                auto subSideBuffer = juce::AudioBuffer<FloatType>(subBuffer.subBuffer.getArrayOfWritePointers() + 2,
                                                                  2, subBuffer.subBuffer.getNumSamples());
                processSubBuffer(subMainBuffer, subSideBuffer);
                subBuffer.pushSubBuffer();
            }
            subBuffer.popBlock(block);
            // ---------------- end sub buffer
        }
        phaseFlipper.process(mainBuffer);
    }

    template<typename FloatType>
    void Controller<FloatType>::processSubBuffer(juce::AudioBuffer<FloatType> &subMainBuffer,
                                                 juce::AudioBuffer<FloatType> &subSideBuffer) {
        fftAnalyzezr.pushPreFFTBuffer(subMainBuffer);
        fftAnalyzezr.pushSideFFTBuffer(subSideBuffer);
        conflictAnalyzer.pushRefBuffer(subSideBuffer);
        if (isEffectON.load()) {
            if (useSolo.load()) {
                processSolo(subMainBuffer, subSideBuffer);
            } else {
                processDynamic(subMainBuffer, subSideBuffer);
                if (currentFtilerStructure == filterStructure::parallel) {
                    processParallelPost(subMainBuffer);
                }
            }
        }
        fftAnalyzezr.pushPostFFTBuffer(subMainBuffer);
        fftAnalyzezr.process();
        conflictAnalyzer.pushMainBuffer(subMainBuffer);
        conflictAnalyzer.process();
    }

    template<typename FloatType>
    void Controller<FloatType>::processSolo(juce::AudioBuffer<FloatType> &subMainBuffer,
                                            juce::AudioBuffer<FloatType> &subSideBuffer) {
        for (size_t i = 0; i < bandNUM; ++i) {
            filters[i].processBypass();
        }
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
    void Controller<FloatType>::processDynamic(juce::AudioBuffer<FloatType> &subMainBuffer,
                                               juce::AudioBuffer<FloatType> &subSideBuffer) {
        autoGain.processPre(subMainBuffer);
        // set auto threshold
        for (size_t i = 0; i < bandNUM; ++i) {
            if (filters[i].getDynamicON()) {
                if (isHistON[i].load()) {
                    const auto depThres =
                            currentThreshold[i].load() + FloatType(40) +
                            static_cast<FloatType>(threshold::range.snapToLegalValue(
                                static_cast<float>(-subHistograms[i].getPercentile(FloatType(0.5)))));
                    filters[i].getCompressor().getComputer().setThreshold(depThres);
                } else {
                    filters[i].getCompressor().getComputer().setThreshold(currentThreshold[i].load());
                }
            }
        }
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
        if (useLR) {
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
        if (useMS) {
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
            if (filters[i].getDynamicON() ) {
                mainFilters[i].setGain(filters[i].getMainFilter().getGain());
                mainFilters[i].setQ(filters[i].getMainFilter().getQ());
                if (isHistON[i].load()) {
                    auto &compressor = filters[i].getCompressor();
                    const auto diff = compressor.getBaseLine() - compressor.getTracker().getMomentaryLoudness();
                    if (diff <= 100) {
                        const auto histIdx = juce::jlimit(0, 79, juce::roundToInt(diff));
                        histograms[i].push(static_cast<size_t>(histIdx));
                        subHistograms[i].push(static_cast<size_t>(histIdx));
                    }
                }
            }
        }
        autoGain.processPost(subMainBuffer);
        outputGain.process(subMainBuffer);
    }

    template<typename FloatType>
    void Controller<FloatType>::processParallelPost(juce::AudioBuffer<FloatType> &subMainBuffer) {
        for (const auto &f : {true, false}) {
            for (size_t i = 0; i < bandNUM; ++i) {
                if (filters[i].getMainFilter().getShouldBeParallel() == f && filterLRs[i].load() == lrType::stereo) {
                    filters[i].processParallelPost(subMainBuffer);
                }
            }
            if (useLR) {
                lrMainSplitter.split(subMainBuffer);
                for (size_t i = 0; i < bandNUM; ++i) {
                    if (filters[i].getMainFilter().getShouldBeParallel() == f) {
                        if (filterLRs[i].load() == lrType::left) {
                            filters[i].processParallelPost(lrMainSplitter.getLBuffer());
                        } else if (filterLRs[i].load() == lrType::right) {
                            filters[i].processParallelPost(lrMainSplitter.getRBuffer());
                        }
                    }
                }
                lrMainSplitter.combine(subMainBuffer);
            }
            if (useMS) {
                msMainSplitter.split(subMainBuffer);
                for (size_t i = 0; i < bandNUM; ++i) {
                    if (filters[i].getMainFilter().getShouldBeParallel() == f) {
                        if (filterLRs[i].load() == lrType::mid) {
                            filters[i].processParallelPost(msMainSplitter.getMBuffer());
                        } else if (filterLRs[i].load() == lrType::side) {
                            filters[i].processParallelPost(msMainSplitter.getSBuffer());
                        }
                    }
                }
                msMainSplitter.combine(subMainBuffer);
            }
        }
    }


    template<typename FloatType>
    void Controller<FloatType>::processBypass() {
        for (size_t i = 0; i < bandNUM; ++i) {
            filters[i].processBypass();
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::setFilterLRs(const lrType::lrTypes x, const size_t idx) {
        filterLRs[idx].store(x);
        // update useLR and useMS
        toUpdateLRs.store(true);

    }

    template<typename FloatType>
    void Controller<FloatType>::setDynamicON(const bool x, size_t idx) {
        filters[idx].setDynamicON(x);
        filters[idx].getMainFilter().setGain(filters[idx].getBaseFilter().getGain(), false);
        filters[idx].getMainFilter().setQ(filters[idx].getBaseFilter().getQ(), true);
    }

    template<typename FloatType>
    void Controller<FloatType>::handleAsyncUpdate() {
        int latency = static_cast<int>(delay.getDelaySamples());
        if (!isZeroLatency.load()) {
            latency += static_cast<int>(subBuffer.getLatencySamples());
        }
        processorRef.setLatencySamples(latency);
    }

    template<typename FloatType>
    std::tuple<FloatType, FloatType> Controller<FloatType>::getSoloFilterParas(
        const zlFilter::FilterType fType, const FloatType freq, const FloatType q) {
        switch (fType) {
            case zlFilter::FilterType::highPass:
            case zlFilter::FilterType::lowShelf: {
                auto soloFreq = static_cast<FloatType>(std::sqrt(1) * std::sqrt(freq));
                auto scale = soloFreq;
                soloFreq = static_cast<FloatType>(std::min(std::max(soloFreq, FloatType(10)), FloatType(20000)));
                auto bw = std::max(std::log2(scale) * 2, FloatType(0.01));
                auto soloQ = 1 / (2 * std::sinh(std::log(FloatType(2)) / 2 * bw));
                soloQ = std::min(std::max(soloQ, FloatType(0.025)), FloatType(25));
                return {soloFreq, soloQ};
            }
            case zlFilter::FilterType::lowPass:
            case zlFilter::FilterType::highShelf: {
                auto soloFreq = static_cast<FloatType>(
                    std::sqrt(subBuffer.getMainSpec().sampleRate / 2) * std::sqrt(freq));
                auto scale = soloFreq / freq;
                soloFreq = static_cast<FloatType>(std::min(std::max(soloFreq, FloatType(10)), FloatType(20000)));
                auto bw = std::max(std::log2(scale) * 2, FloatType(0.01));
                auto soloQ = 1 / (2 * std::sinh(std::log(FloatType(2)) / 2 * bw));
                soloQ = std::min(std::max(soloQ, FloatType(0.025)), FloatType(25));
                return {soloFreq, soloQ};
            }
            case zlFilter::FilterType::tiltShelf: {
                return {freq, FloatType(0.025)};
            }
            case zlFilter::FilterType::peak:
            case zlFilter::FilterType::notch:
            case zlFilter::FilterType::bandPass:
            case zlFilter::FilterType::bandShelf:
            default: {
                return {freq, q};
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::setSolo(const size_t idx, const bool isSide) {
        FloatType freq, q;
        if (!isSide) {
            const auto &f{filters[idx].getMainFilter()};
            std::tie(freq, q) = getSoloFilterParas(
                f.getFilterType(), f.getFreq(), f.getQ());
        } else {
            const auto &f{filters[idx].getSideFilter()};
            std::tie(freq, q) = getSoloFilterParas(
                f.getFilterType(), f.getFreq(), f.getQ());
        }
        soloFilter.setFreq(freq, false);
        soloFilter.setQ(q, true);

        soloIdx.store(idx);
        soloSide.store(isSide);

        isSoloUpdated.store(true);
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
            subHistograms[idx].reset(FloatType(12.5));
        }
        isHistON[idx].store(isLearning);
    }

    template<typename FloatType>
    void Controller<FloatType>::setLookAhead(const FloatType x) {
        delay.setDelaySeconds(x / static_cast<FloatType>(1000));
        triggerAsyncUpdate();
    }

    template<typename FloatType>
    void Controller<FloatType>::setRMS(const FloatType x) {
        const auto rmsMs = x / static_cast<FloatType>(1000);
        for (auto &f: filters) {
            f.getCompressor().getTracker().setMomentarySeconds(rmsMs);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateFilterStructure() {
        switch (currentFtilerStructure) {
            case filterStructure::minimum:
            case filterStructure::matched:
            case filterStructure::mixed: {
                for (auto &f: filters) {
                    f.setFilterStructure(zlFilter::FilterStructure::iir);
                }
                break;
            }
            case filterStructure::svf: {
                for (auto &f: filters) {
                    f.setFilterStructure(zlFilter::FilterStructure::svf);
                }
                break;
            }
            case filterStructure::parallel: {
                for (auto &f: filters) {
                    f.setFilterStructure(zlFilter::FilterStructure::parallel);
                }
                break;
            }
            case filterStructure::linear: {
            }
        }
    }

    template
    class Controller<float>;

    template
    class Controller<double>;
}
