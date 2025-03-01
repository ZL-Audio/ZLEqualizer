// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "controller.hpp"

namespace zlDSP {
    template<typename FloatType>
    Controller<FloatType>::Controller(juce::AudioProcessor &processor, const size_t fftOrder)
        : processorRef(processor),
          fftAnalyzer(fftOrder), conflictAnalyzer(fftOrder), matchAnalyzer(13) {
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

        for (auto &f: filters) {
            f.getTracker().setMaximumMomentarySeconds(static_cast<FloatType>(zlDSP::dynRMS::range.end / 1000.f));
        }

        juce::dsp::ProcessSpec subSpec{sampleRate.load(), subBuffer.getSubSpec().maximumBlockSize, 2};
        for (auto &f: filters) {
            f.prepare(subSpec);
        }

        prototypeCorrections[0].prepare(subSpec);
        for (size_t i = 1; i < 5; ++i) {
            prototypeCorrections[i].prepare(juce::dsp::ProcessSpec{subSpec.sampleRate, subSpec.maximumBlockSize, 1});
        }
        prototypeW1.resize(prototypeCorrections[0].getCorrectionSize());
        prototypeW2.resize(prototypeCorrections[0].getCorrectionSize());
        zlFilter::calculateWsForPrototype<FloatType>(prototypeW1);
        zlFilter::calculateWsForBiquad<FloatType>(prototypeW2);

        mixedCorrections[0].prepare(subSpec);
        for (size_t i = 1; i < 5; ++i) {
            mixedCorrections[i].prepare(juce::dsp::ProcessSpec{subSpec.sampleRate, subSpec.maximumBlockSize, 1});
        }
        mixedW1.resize(mixedCorrections[0].getCorrectionSize());
        mixedW2.resize(mixedCorrections[0].getCorrectionSize());
        zlFilter::calculateWsForPrototype<FloatType>(mixedW1);
        zlFilter::calculateWsForBiquad<FloatType>(mixedW2);

        linearFilters[0].prepare(subSpec);
        for (size_t i = 1; i < 5; ++i) {
            linearFilters[i].prepare(juce::dsp::ProcessSpec{subSpec.sampleRate, subSpec.maximumBlockSize, 1});
        }
        linearW1.resize(linearFilters[0].getCorrectionSize());
        zlFilter::calculateWsForPrototype<FloatType>(linearW1);

        for (auto &f: mainIIRs) {
            f.prepare(subSpec.sampleRate);
            f.prepareResponseSize(mixedCorrections[0].getCorrectionSize());
        }
        for (auto &f: mainIdeals) {
            f.prepare(subSpec.sampleRate);
            f.prepareResponseSize(linearFilters[0].getCorrectionSize());
        }

        soloFilter.setFilterType(zlFilter::FilterType::bandPass);
        soloFilter.prepare(subSpec);

        lrMainSplitter.prepare(subSpec);
        lrSideSplitter.prepare(subSpec);
        msMainSplitter.prepare(subSpec);
        msSideSplitter.prepare(subSpec);
        outputGain.prepare(subSpec);
        autoGain.prepare(subSpec);
        for (auto &g: compensationGains) {
            g.prepare(subSpec);
        }
        fftAnalyzer.prepare(subSpec);
        fftAnalyzer.getPreDelay().setMaximumDelayInSamples(linearFilters[0].getLatency() * 3 + 10);
        fftAnalyzer.getPreDelay().prepare(subSpec);
        fftAnalyzer.getSideDelay().setMaximumDelayInSamples(linearFilters[0].getLatency() * 3 + 10);
        fftAnalyzer.getSideDelay().prepare(subSpec);

        conflictAnalyzer.prepare(subSpec);
        conflictAnalyzer.getSideDelay().setMaximumDelayInSamples(linearFilters[0].getLatency() * 3 + 10);
        conflictAnalyzer.getSideDelay().prepare(subSpec);

        matchAnalyzer.prepare(subSpec);

        for (auto &t: trackers) {
            t.prepare(subSpec.sampleRate);
        }

        toUpdateLRs.store(true);
        triggerAsyncUpdate();
    }

    template<typename FloatType>
    void Controller<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        currentIsEditorOn = isEditorOn.load();
        if (mFilterStructure.load() != currentFilterStructure) {
            currentFilterStructure = mFilterStructure.load();
            updateFilterStructure();
            toUpdateLRs.store(true);
        }
        if (toUpdateDynamicON.exchange(false)) {
            updateDynamicONs();
        }
        if (toUpdateLRs.exchange(false)) {
            updateLRs();
            updateTrackersON();
            updateCorrections();
            toUpdateSgc.store(true);
        }
        if (toUpdateBypass.exchange(false)) {
            for (size_t i = 0; i < bandNUM; ++i) {
                currentIsBypass[i] = isBypass[i].load();
            }
            updateCorrections();
            toUpdateSgc.store(true);
        }
        if (currentIsSgcON != isSgcON.load()) {
            currentIsSgcON = isSgcON.load();
            if (!currentIsSgcON) {
                for (auto &cG: compensationGains) {
                    cG.setGainLinear(FloatType(1));
                }
            } else {
                toUpdateSgc.store(true);
            }
        }
        if (currentIsSgcON) {
            if (toUpdateSgc.exchange(false)) {
                updateSgcValues();
            }
        }
        if (toUpdateSolo.exchange(false)) {
            currentUseSolo = useSolo.load();
            updateSolo();
        }
        if (toUpdateDynRelSide.exchange(false)) {
            updateDynRelSide();
            updateTrackersON();
        }
        if (toUpdateHist.exchange(false)) {
            updateHistograms();
        }
        currentIsEffectON = isEffectON.load();

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
        if (currentIsEditorOn) {
            fftAnalyzer.pushPreFFTBuffer(subMainBuffer);
            matchAnalyzer.process(subMainBuffer, subSideBuffer);
        }

        if (currentIsEffectON) {
            if (currentUseSolo) {
                processSubBufferOnOff<true>(subMainBuffer, subSideBuffer);
                processSolo(subMainBuffer, subSideBuffer);
            } else {
                processSubBufferOnOff<false>(subMainBuffer, subSideBuffer);
            }
        } else {
            processSubBufferOnOff<true>(subMainBuffer, subSideBuffer);
        }

        if (currentIsEditorOn) {
            fftAnalyzer.pushSideFFTBuffer(subSideBuffer);
            fftAnalyzer.pushPostFFTBuffer(subMainBuffer);
            fftAnalyzer.process();
            conflictAnalyzer.pushMainBuffer(subMainBuffer);
            conflictAnalyzer.pushRefBuffer(subSideBuffer);
            conflictAnalyzer.process();
        }
    }

    template<typename FloatType>
    template<bool isBypassed>
    void Controller<FloatType>::processSubBufferOnOff(juce::AudioBuffer<FloatType> &subMainBuffer,
                                                      juce::AudioBuffer<FloatType> &subSideBuffer) {
        if (currentFilterStructure == filterStructure::linear) {
            processLinear<isBypassed>(subMainBuffer);
        } else {
            autoGain.processPre(subMainBuffer);
            processDynamic<isBypassed>(subMainBuffer, subSideBuffer);
            if (currentFilterStructure == filterStructure::parallel) {
                processParallelPost<isBypassed>(subMainBuffer, subSideBuffer);
            }
            autoGain.template processPost<isBypassed>(subMainBuffer);
            if (currentFilterStructure == filterStructure::matched) {
                processPrototypeCorrection<isBypassed>(subMainBuffer);
            } else if (currentFilterStructure == filterStructure::mixed) {
                processMixedCorrection<isBypassed>(subMainBuffer);
            }
        }
        outputGain.template process<isBypassed>(subMainBuffer);
    }

    template<typename FloatType>
    void Controller<FloatType>::processSolo(juce::AudioBuffer<FloatType> &subMainBuffer,
                                            juce::AudioBuffer<FloatType> &subSideBuffer) {
        if (currentSoloSide) {
            subMainBuffer.makeCopyOf(subSideBuffer, true);
        }
        soloFilter.processPre(subMainBuffer);
        switch (currentFilterLRs[currentSoloIdx]) {
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
    template<bool isBypassed>
    void Controller<FloatType>::processDynamic(juce::AudioBuffer<FloatType> &subMainBuffer,
                                               juce::AudioBuffer<FloatType> &subSideBuffer) {
        // set auto threshold
        if (!isBypassed) {
            for (size_t idx = 0; idx < dynamicONIndices.size(); ++idx) {
                const auto i = dynamicONIndices[idx];
                if (currentIsHistON[i]) {
                    const auto depThres =
                            currentThreshold[i].load() + FloatType(40) +
                            static_cast<FloatType>(threshold::range.snapToLegalValue(
                                static_cast<float>(-subHistograms[i].getPercentile(FloatType(0.5)))));
                    filters[i].getComputer().setThreshold(depThres);
                } else {
                    filters[i].getComputer().setThreshold(currentThreshold[i].load());
                }
            }
        }
        // stereo filters process
        processDynamicLRMSTrackers<0>(subSideBuffer);
        processDynamicLRMS<isBypassed, 0, 0>(subMainBuffer, subSideBuffer, subSideBuffer);
        // LR filters process
        if (useLR) {
            lrMainSplitter.split(subMainBuffer);
            lrSideSplitter.split(subSideBuffer);
            processDynamicLRMSTrackers<1>(lrSideSplitter.getLBuffer());
            processDynamicLRMSTrackers<2>(lrSideSplitter.getRBuffer());
            processDynamicLRMS<isBypassed, 1, 2>(lrMainSplitter.getLBuffer(),
                                                 lrSideSplitter.getLBuffer(), lrSideSplitter.getRBuffer());
            processDynamicLRMS<isBypassed, 2, 1>(lrMainSplitter.getRBuffer(),
                                                 lrSideSplitter.getRBuffer(), lrSideSplitter.getLBuffer());
            lrMainSplitter.combine(subMainBuffer);
        }
        // MS filters process
        if (useMS) {
            msMainSplitter.split(subMainBuffer);
            msSideSplitter.split(subSideBuffer);
            processDynamicLRMSTrackers<3>(msSideSplitter.getMBuffer());
            processDynamicLRMSTrackers<4>(msSideSplitter.getSBuffer());
            processDynamicLRMS<isBypassed, 3, 4>(msMainSplitter.getMBuffer(),
                                                 msSideSplitter.getMBuffer(), msSideSplitter.getSBuffer());
            processDynamicLRMS<isBypassed, 4, 3>(msMainSplitter.getSBuffer(),
                                                 msSideSplitter.getSBuffer(), msSideSplitter.getMBuffer());
            msMainSplitter.combine(subMainBuffer);
        }
        // set main filter gain & Q and update histograms
        if (!isBypassed) {
            for (size_t idx = 0; idx < dynamicONIndices.size(); ++idx) {
                const auto i = dynamicONIndices[idx];
                mainIdeals[i].setGain(filters[i].getMainFilter().template getGain<false>());
                mainIdeals[i].setQ(filters[i].getMainFilter().template getQ<false>());
                mainIIRs[i].setGain(filters[i].getMainFilter().template getGain<false>());
                mainIIRs[i].setQ(filters[i].getMainFilter().template getQ<false>());
                if (currentIsHistON[i]) {
                    const auto diff = filters[i].getBaseLine() - filters[i].getTracker().getMomentaryLoudness();
                    if (diff <= 100) {
                        const auto histIdx = juce::jlimit(0, 79, juce::roundToInt(diff));
                        histograms[i].push(static_cast<size_t>(histIdx));
                        subHistograms[i].push(static_cast<size_t>(histIdx));
                        atomicHistograms[i].sync(histograms[i]);
                    }
                } else if (currentIsEditorOn) {
                    sideLoudness[i].store(filters[i].getTracker().getMomentaryLoudness() - filters[i].getBaseLine());
                }
            }
        }
    }

    template<typename FloatType>
    template<size_t lrIdx>
    void Controller<FloatType>::processDynamicLRMSTrackers(juce::AudioBuffer<FloatType> &subSideBuffer) {
        auto &tracker{trackers[lrIdx]};
        if (useTrackers[lrIdx]) {
            tracker.processBufferRMS(subSideBuffer);
            trackerBaselines[lrIdx] = tracker.getMomentaryLoudness();
            if (trackerBaselines[lrIdx] <= tracker.minusInfinityDB + FloatType(1)) {
                trackerBaselines[lrIdx] = tracker.minusInfinityDB * FloatType(0.5);
            }
        }
    }

    template<typename FloatType>
    template<bool isBypassed, size_t lrIdx1, size_t lrIdx2>
    void Controller<FloatType>::processDynamicLRMS(juce::AudioBuffer<FloatType> &subMainBuffer,
                                                   juce::AudioBuffer<FloatType> &subSideBuffer1,
                                                   juce::AudioBuffer<FloatType> &subSideBuffer2) {
        const auto &indices{filterLRIndices[lrIdx1]};
        for (size_t idx = 0; idx < indices.size(); ++idx) {
            const auto i = indices[idx];
            const auto baseLine = sideSwaps[i] ? trackerBaselines[lrIdx2] : trackerBaselines[lrIdx1];
            if (currentDynRelatives[i]) {
                filters[i].setBaseLine(baseLine);
            } else {
                filters[i].setBaseLine(0);
            }
            juce::AudioBuffer<FloatType> &subSideBuffer = sideSwaps[i] ? subSideBuffer2 : subSideBuffer1;
            if (currentIsBypass[i] || isBypassed) {
                filters[i].template process<true>(subMainBuffer, subSideBuffer);
            } else {
                filters[i].template process<false>(subMainBuffer, subSideBuffer);
            }
        }
        if (currentIsSgcON && currentFilterStructure != filterStructure::parallel) {
            compensationGains[lrIdx1].template process<isBypassed>(subMainBuffer);
        }
    }

    template<typename FloatType>
    template<bool isBypassed>
    void Controller<FloatType>::processParallelPost(juce::AudioBuffer<FloatType> &subMainBuffer,
                                                    juce::AudioBuffer<FloatType> &subSideBuffer) {
        // add parallel filters first
        processParallelPostLRMS<isBypassed>(0, true, subMainBuffer, subSideBuffer);
        if (useLR) {
            processParallelPostLRMS<isBypassed>(1, true, lrMainSplitter.getLBuffer(), lrSideSplitter.getLBuffer());
            processParallelPostLRMS<isBypassed>(2, true, lrMainSplitter.getRBuffer(), lrSideSplitter.getRBuffer());
            lrMainSplitter.combine(subMainBuffer);
        }
        if (useMS) {
            processParallelPostLRMS<isBypassed>(3, true, msMainSplitter.getMBuffer(), msSideSplitter.getMBuffer());
            processParallelPostLRMS<isBypassed>(4, true, msMainSplitter.getSBuffer(), msSideSplitter.getSBuffer());
            msMainSplitter.combine(subMainBuffer);
        }
        processParallelPostLRMS<isBypassed>(0, false, subMainBuffer, subSideBuffer);
        if (currentIsSgcON) {
            compensationGains[0].template process<isBypassed>(subMainBuffer);
        }
        if (useLR) {
            lrMainSplitter.split(subMainBuffer);
            processParallelPostLRMS<isBypassed>(1, false, lrMainSplitter.getLBuffer(), lrSideSplitter.getLBuffer());
            processParallelPostLRMS<isBypassed>(2, false, lrMainSplitter.getRBuffer(), lrSideSplitter.getRBuffer());
            if (currentIsSgcON) {
                compensationGains[1].template process<isBypassed>(lrMainSplitter.getLBuffer());
                compensationGains[2].template process<isBypassed>(lrMainSplitter.getRBuffer());
            }
            lrMainSplitter.combine(subMainBuffer);
        }
        if (useMS) {
            msMainSplitter.split(subMainBuffer);
            processParallelPostLRMS<isBypassed>(3, false, msMainSplitter.getMBuffer(), msSideSplitter.getMBuffer());
            processParallelPostLRMS<isBypassed>(4, false, msMainSplitter.getSBuffer(), msSideSplitter.getSBuffer());
            if (currentIsSgcON) {
                compensationGains[3].template process<isBypassed>(msMainSplitter.getMBuffer());
                compensationGains[4].template process<isBypassed>(msMainSplitter.getSBuffer());
            }
            msMainSplitter.combine(subMainBuffer);
        }
    }

    template<typename FloatType>
    template<bool isBypassed>
    void Controller<FloatType>::processParallelPostLRMS(const size_t lrIdx, const bool shouldParallel,
                                                        juce::AudioBuffer<FloatType> &subMainBuffer,
                                                        juce::AudioBuffer<FloatType> &subSideBuffer) {
        const auto &indices{filterLRIndices[lrIdx]};
        for (size_t idx = 0; idx < indices.size(); ++idx) {
            const auto i = indices[idx];
            if (filters[i].getMainFilter().getShouldBeParallel() == shouldParallel) {
                if (currentIsBypass[i] || isBypassed) {
                    filters[i].template processParallelPost<true>(subMainBuffer, subSideBuffer);
                } else {
                    filters[i].template processParallelPost<false>(subMainBuffer, subSideBuffer);
                }
            }
        }
    }

    template<typename FloatType>
    template<bool isBypassed>
    void Controller<FloatType>::processPrototypeCorrection(juce::AudioBuffer<FloatType> &subMainBuffer) {
        prototypeCorrections[0].template process<isBypassed>(subMainBuffer);
        if (useLR) {
            lrMainSplitter.split(subMainBuffer);
            prototypeCorrections[1].template process<isBypassed>(lrMainSplitter.getLBuffer());
            prototypeCorrections[2].template process<isBypassed>(lrMainSplitter.getRBuffer());
            lrMainSplitter.combine(subMainBuffer);
        }
        if (useMS) {
            msMainSplitter.split(subMainBuffer);
            prototypeCorrections[3].template process<isBypassed>(msMainSplitter.getMBuffer());
            prototypeCorrections[4].template process<isBypassed>(msMainSplitter.getSBuffer());
            msMainSplitter.combine(subMainBuffer);
        }
    }

    template<typename FloatType>
    template<bool isBypassed>
    void Controller<FloatType>::processMixedCorrection(juce::AudioBuffer<FloatType> &subMainBuffer) {
        mixedCorrections[0].template process<isBypassed>(subMainBuffer);
        if (useLR) {
            lrMainSplitter.split(subMainBuffer);
            mixedCorrections[1].template process<isBypassed>(lrMainSplitter.getLBuffer());
            mixedCorrections[2].template process<isBypassed>(lrMainSplitter.getRBuffer());
            lrMainSplitter.combine(subMainBuffer);
        }
        if (useMS) {
            msMainSplitter.split(subMainBuffer);
            mixedCorrections[3].template process<isBypassed>(msMainSplitter.getMBuffer());
            mixedCorrections[4].template process<isBypassed>(msMainSplitter.getSBuffer());
            msMainSplitter.combine(subMainBuffer);
        }
    }

    template<typename FloatType>
    template<bool isBypassed>
    void Controller<FloatType>::processLinear(juce::AudioBuffer<FloatType> &subMainBuffer) {
        linearFilters[0].template process<isBypassed>(subMainBuffer);
        if (currentIsSgcON) {
            compensationGains[0].template process<isBypassed>(subMainBuffer);
        }
        if (useLR) {
            lrMainSplitter.split(subMainBuffer);
            linearFilters[1].template process<isBypassed>(lrMainSplitter.getLBuffer());
            linearFilters[2].template process<isBypassed>(lrMainSplitter.getRBuffer());
            if (currentIsSgcON) {
                compensationGains[1].template process<isBypassed>(lrMainSplitter.getLBuffer());
                compensationGains[2].template process<isBypassed>(lrMainSplitter.getRBuffer());
            }
            lrMainSplitter.combine(subMainBuffer);
        }
        if (useMS) {
            msMainSplitter.split(subMainBuffer);
            linearFilters[3].template process<isBypassed>(msMainSplitter.getMBuffer());
            linearFilters[4].template process<isBypassed>(msMainSplitter.getSBuffer());
            if (currentIsSgcON) {
                compensationGains[3].template process<isBypassed>(msMainSplitter.getMBuffer());
                compensationGains[4].template process<isBypassed>(msMainSplitter.getSBuffer());
            }
            msMainSplitter.combine(subMainBuffer);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::setFilterLRs(const lrType::lrTypes x, const size_t idx) {
        filterLRs[idx].store(x);
        toUpdateLRs.store(true);
    }

    template<typename FloatType>
    void Controller<FloatType>::setDynamicON(const bool x, size_t idx) {
        const auto bGain = bFilters[idx].getGain();
        const auto bQ = bFilters[idx].getQ();

        filters[idx].setDynamicON(x);
        filters[idx].getMainFilter().setGain(bFilters[idx].getGain());
        filters[idx].getMainFilter().setQ(bFilters[idx].getQ());

        mainIIRs[idx].setGain(bGain);
        mainIIRs[idx].setQ(bQ);
        mainIdeals[idx].setGain(bGain);
        mainIdeals[idx].setQ(bQ);

        toUpdateDynamicON.store(true);
    }

    template<typename FloatType>
    void Controller<FloatType>::handleAsyncUpdate() {
        int currentLatency = static_cast<int>(delay.getDelaySamples());
        if (!isZeroLatency.load()) {
            currentLatency += static_cast<int>(subBuffer.getLatencySamples());
        }
        currentLatency += latency.load();
        processorRef.setLatencySamples(currentLatency);
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
        soloIdx.store(idx);
        soloSide.store(isSide);

        useSolo.store(true);
        toUpdateSolo.store(true);
    }

    template<typename FloatType>
    void Controller<FloatType>::clearSolo(const size_t idx, const bool isSide) {
        if (idx == soloIdx.load() && isSide == soloSide.load()) {
            useSolo.store(false);
            toUpdateSolo.store(true);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::setRelative(const size_t idx, const bool isRelative) {
        dynRelatives[idx].store(isRelative);
        toUpdateDynRelSide.store(true);
    }

    template<typename FloatType>
    void Controller<FloatType>::setSideSwap(const size_t idx, const bool isSwap) {
        sideSwaps[idx].store(isSwap);
        toUpdateDynRelSide.store(true);
    }

    template<typename FloatType>
    void Controller<FloatType>::updateDynamicONs() {
        dynamicONIndices.clear();
        for (size_t i = 0; i < bandNUM; ++i) {
            if (filters[i].getDynamicON()) {
                dynamicONIndices.push(i);
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateLRs() {
        useLR = false;
        useMS = false;
        for (auto &x: filterLRIndices) {
            x.clear();
        }
        for (size_t i = 0; i < bandNUM; ++i) {
            if (isActive[i].load()) {
                currentFilterLRs[i] = filterLRs[i].load();
                switch (currentFilterLRs[i]) {
                    case lrType::stereo: {
                        filterLRIndices[0].push(i);
                        break;
                    }
                    case lrType::left: {
                        filterLRIndices[1].push(i);
                        useLR = true;
                        break;
                    }
                    case lrType::right: {
                        filterLRIndices[2].push(i);
                        useLR = true;
                        break;
                    }
                    case lrType::mid: {
                        filterLRIndices[3].push(i);
                        useMS = true;
                        break;
                    }
                    case lrType::side: {
                        filterLRIndices[4].push(i);
                        useMS = true;
                        break;
                    }
                }
            }
        }
        int newLatency = 0;
        switch (currentFilterStructure) {
            case filterStructure::minimum:
            case filterStructure::svf:
            case filterStructure::parallel: {
                newLatency = 0;
                break;
            }
            case filterStructure::matched: {
                const auto singleLatency = prototypeCorrections[0].getLatency();
                newLatency = singleLatency
                             + static_cast<int>(useLR) * singleLatency
                             + static_cast<int>(useMS) * singleLatency;
                break;
            }
            case filterStructure::mixed: {
                const auto singleLatency = mixedCorrections[0].getLatency();
                newLatency = singleLatency
                             + static_cast<int>(useLR) * singleLatency
                             + static_cast<int>(useMS) * singleLatency;
                break;
            }
            case filterStructure::linear: {
                const auto singleLatency = linearFilters[0].getLatency();
                newLatency = singleLatency
                             + static_cast<int>(useLR) * singleLatency
                             + static_cast<int>(useMS) * singleLatency;
                break;
            }
        }
        if (newLatency != latency.load()) {
            const auto delayInSeconds = static_cast<FloatType>(newLatency) / static_cast<FloatType>(sampleRate.load());
            fftAnalyzer.getPreDelay().setDelaySeconds(delayInSeconds);
            fftAnalyzer.getSideDelay().setDelaySeconds(delayInSeconds);
            conflictAnalyzer.getSideDelay().setDelaySeconds(delayInSeconds);
            latency.store(newLatency);
            triggerAsyncUpdate();
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateTrackersON() {
        std::fill(useTrackers.begin(), useTrackers.end(), false);
        for (size_t idx = 0; idx < 5; ++idx) {
            const auto &indices{filterLRIndices[idx]};
            for (size_t i = 0; i < indices.size(); ++i) {
                if (currentDynRelatives[i]) {
                    useTrackers[idx] = true;
                    break;
                }
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::setLearningHistON(const size_t idx, const bool isLearning) {
        isHistON[idx].store(isLearning);
        toUpdateHist.store(true);
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
            f.getTracker().setMomentarySeconds(rmsMs);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateFilterStructure() {
        switch (currentFilterStructure) {
            case filterStructure::minimum: {
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
            case filterStructure::matched: {
                for (auto &f: filters) {
                    f.setFilterStructure(zlFilter::FilterStructure::iir);
                }
                for (auto &f: mainIIRs) {
                    f.setToUpdate();
                }
                for (auto &f: mainIdeals) {
                    f.setToUpdate();
                }
                for (auto &c: prototypeCorrections) {
                    c.reset();
                }
                break;
            }
            case filterStructure::mixed: {
                for (auto &f: filters) {
                    f.setFilterStructure(zlFilter::FilterStructure::iir);
                }
                for (auto &f: mainIIRs) {
                    f.setToUpdate();
                }
                for (auto &f: mainIdeals) {
                    f.setToUpdate();
                }
                for (auto &c: mixedCorrections) {
                    c.reset();
                }
                break;
            }
            case filterStructure::linear: {
                for (auto &f: mainIdeals) {
                    f.setToUpdate();
                }
                for (auto &c: linearFilters) {
                    c.reset();
                }
                for (size_t idx = 0; idx < bandNUM; ++idx) {
                    const auto bGain = bFilters[idx].getGain();
                    const auto bQ = bFilters[idx].getQ();
                    mainIIRs[idx].setGain(bGain);
                    mainIIRs[idx].setQ(bQ);
                    mainIdeals[idx].setGain(bGain);
                    mainIdeals[idx].setQ(bQ);
                }
                break;
            }
        }
        for (size_t idx = 0; idx < bandNUM; ++idx) {
            const auto bGain = bFilters[idx].getGain();
            const auto bQ = bFilters[idx].getQ();
            filters[idx].getMainFilter().setGain(bGain);
            filters[idx].getMainFilter().setQ(bQ);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateSgcValues() {
        for (size_t lr = 0; lr < 5; ++lr) {
            auto &indices{filterLRIndices[lr]};
            FloatType currentSgc{FloatType(1)};
            for (size_t idx = 0; idx < indices.size(); ++idx) {
                const auto i = indices[idx];
                if (!currentIsBypass[i]) {
                    currentSgc *= compensations[i].getGain();
                }
            }
            compensationGains[lr].setGainLinear(currentSgc);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateCorrections() {
        if (currentFilterStructure == filterStructure::matched) {
            for (auto &c: prototypeCorrections) {
                c.setToUpdate();
            }
        } else if (currentFilterStructure == filterStructure::mixed) {
            for (auto &c: mixedCorrections) {
                c.setToUpdate();
            }
        } else if (currentFilterStructure == filterStructure::linear) {
            for (auto &c: linearFilters) {
                c.setToUpdate();
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateSolo() {
        if (currentUseSolo) {
            currentSoloIdx = soloIdx.load();
            currentSoloSide = soloSide.load();
        } else {
            soloFilter.setToRest();
            return;
        }

        FloatType freq, q;
        if (!currentSoloSide) {
            const auto &f{bFilters[currentSoloIdx]};
            std::tie(freq, q) = getSoloFilterParas(
                f.getFilterType(), f.getFreq(), f.getQ());
        } else {
            const auto &f{filters[currentSoloIdx].getSideFilter()};
            std::tie(freq, q) = getSoloFilterParas(
                f.getFilterType(), f.getFreq(), f.getQ());
        }
        soloFilter.setFreq(freq);
        soloFilter.setQ(q);
    }

    template<typename FloatType>
    void Controller<FloatType>::updateDynRelSide() {
        for (size_t i = 0; i < bandNUM; ++i) {
            if (currentDynRelatives[i] != dynRelatives[i].load() || currentSideSwaps[i] != sideSwaps[i].load()) {
                currentDynRelatives[i] = dynRelatives[i].load();
                currentSideSwaps[i] = sideSwaps[i].load();
                histograms[i].reset(FloatType(12.5));
                subHistograms[i].reset(FloatType(12.5));
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateHistograms() {
        for (size_t i = 0; i < bandNUM; ++i) {
            if (currentIsHistON[i] != isHistON[i].load()) {
                currentIsHistON[i] = isHistON[i].load();
                histograms[i].reset(FloatType(12.5));
                subHistograms[i].reset(FloatType(12.5));
            }
        }
    }

    template
    class Controller<float>;

    template
    class Controller<double>;
}
