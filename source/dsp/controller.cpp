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
        for (auto &f: mainIIRs) {
            f.prepare(subSpec.sampleRate);
        }
        for (auto &f: mainIdeals) {
            f.prepare(subSpec.sampleRate);
        }
        prototypeCorrections[0].prepare(subSpec);
        for (size_t i = 1; i < 5; ++i) {
            prototypeCorrections[i].prepare(juce::dsp::ProcessSpec{subSpec.sampleRate, subSpec.maximumBlockSize, 1});
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
        conflictAnalyzer.prepare(subSpec);
        for (auto &t: trackers) {
            t.prepare(subSpec);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        if (mFilterStructure.load() != currentFilterStructure) {
            currentFilterStructure = mFilterStructure.load();
            updateFilterStructure();
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
        }
        if (currentIsSgcON) {
            if (toUpdateSgc.exchange(false)) {
                updateSgcValues();
            }
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
        fftAnalyzer.pushPreFFTBuffer(subMainBuffer);
        fftAnalyzer.pushSideFFTBuffer(subSideBuffer);
        conflictAnalyzer.pushRefBuffer(subSideBuffer);
        if (isEffectON.load()) {
            if (useSolo.load()) {
                processSolo(subMainBuffer, subSideBuffer);
            } else {
                processDynamic(subMainBuffer, subSideBuffer);
                if (currentFilterStructure == filterStructure::parallel) {
                    processParallelPost(subMainBuffer);
                }
            }
        }
        fftAnalyzer.pushPostFFTBuffer(subMainBuffer);
        fftAnalyzer.process();
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
        for (size_t idx = 0; idx < dynamicONIndices.size(); ++idx) {
            const auto i = dynamicONIndices[idx];
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
        // stereo filters process
        processDynamicLRMS(0, subMainBuffer, subSideBuffer);
        // LR filters process
        if (useLR) {
            lrMainSplitter.split(subMainBuffer);
            lrSideSplitter.split(subSideBuffer);
            processDynamicLRMS(1, lrMainSplitter.getLBuffer(),
                               lrSideSplitter.getLBuffer());
            processDynamicLRMS(2, lrMainSplitter.getRBuffer(),
                               lrSideSplitter.getRBuffer());
            lrMainSplitter.combine(subMainBuffer);
        }
        // MS filters process
        if (useMS) {
            msMainSplitter.split(subMainBuffer);
            msSideSplitter.split(subSideBuffer);
            processDynamicLRMS(3, msMainSplitter.getMBuffer(),
                               msSideSplitter.getMBuffer());
            processDynamicLRMS(4, msMainSplitter.getSBuffer(),
                               msSideSplitter.getSBuffer());
            msMainSplitter.combine(subMainBuffer);
        }
        // set main filter gain & Q and update histograms
        for (size_t idx = 0; idx < dynamicONIndices.size(); ++idx) {
            const auto i = dynamicONIndices[idx];
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
        autoGain.processPost(subMainBuffer);
        outputGain.process(subMainBuffer);
    }

    template<typename FloatType>
    void Controller<FloatType>::processDynamicLRMS(const size_t lrIdx,
                                                   juce::AudioBuffer<FloatType> &subMainBuffer,
                                                   juce::AudioBuffer<FloatType> &subSideBuffer) {
        auto &tracker{trackers[lrIdx]};
        const auto &indices{filterLRIndices[lrIdx]};
        FloatType baseLine = 0;
        if (useTrackers[lrIdx]) {
            tracker.process(subSideBuffer);
            baseLine = tracker.getMomentaryLoudness();
            if (baseLine <= tracker.minusInfinityDB + 1) {
                baseLine = tracker.minusInfinityDB * FloatType(0.5);
            }
        }
        for (size_t idx = 0; idx < indices.size(); ++idx) {
            const auto i = indices[idx];
            if (dynRelatives[i].load()) {
                filters[i].getCompressor().setBaseLine(baseLine);
            } else {
                filters[i].getCompressor().setBaseLine(0);
            }
            if (currentIsBypass[i]) {
                filters[i].template process<true>(subMainBuffer, subSideBuffer);
            } else {
                filters[i].template process<false>(subMainBuffer, subSideBuffer);
            }
        }
        if (currentIsSgcON && currentFilterStructure != filterStructure::parallel) {
            compensationGains[lrIdx].process(subMainBuffer);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::processParallelPost(juce::AudioBuffer<FloatType> &subMainBuffer) {
        // add parallel filters first
        processParallelPostLRMS(0, true, subMainBuffer);
        if (useLR) {
            processParallelPostLRMS(1, true, lrMainSplitter.getLBuffer());
            processParallelPostLRMS(2, true, lrMainSplitter.getRBuffer());
            lrMainSplitter.combine(subMainBuffer);
        }
        if (useMS) {
            processParallelPostLRMS(3, true, msMainSplitter.getMBuffer());
            processParallelPostLRMS(4, true, msMainSplitter.getSBuffer());
            msMainSplitter.combine(subMainBuffer);
        }
        processParallelPostLRMS(0, false, subMainBuffer);
        if (currentIsSgcON) {
            compensationGains[0].process(subMainBuffer);
        }
        if (useLR) {
            processParallelPostLRMS(1, false, lrMainSplitter.getLBuffer());
            processParallelPostLRMS(2, false, lrMainSplitter.getRBuffer());
            compensationGains[1].process(lrMainSplitter.getLBuffer());
            compensationGains[2].process(lrMainSplitter.getRBuffer());
            lrMainSplitter.combine(subMainBuffer);
        }
        if (useMS) {
            processParallelPostLRMS(3, false, msMainSplitter.getMBuffer());
            processParallelPostLRMS(4, false, msMainSplitter.getSBuffer());
            compensationGains[3].process(msMainSplitter.getMBuffer());
            compensationGains[4].process(msMainSplitter.getSBuffer());
            msMainSplitter.combine(subMainBuffer);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::processParallelPostLRMS(const size_t lrIdx, const bool shouldParallel,
                                                        juce::AudioBuffer<FloatType> &subMainBuffer) {
        const auto &indices{filterLRIndices[lrIdx]};
        for (size_t idx = 0; idx < indices.size(); ++idx) {
            const auto i = indices[idx];
            if (filters[i].getMainFilter().getShouldBeParallel() == shouldParallel) {
                if (currentIsBypass[i]) {
                    filters[i].template processParallelPost<true>(subMainBuffer);
                } else {
                    filters[i].template processParallelPost<false>(subMainBuffer);
                }
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
        toUpdateLRs.store(true);
    }

    template<typename FloatType>
    void Controller<FloatType>::setDynamicON(const bool x, size_t idx) {
        filters[idx].setDynamicON(x);
        filters[idx].getMainFilter().template setGain<false>(bFilters[idx].getGain());
        filters[idx].getMainFilter().template setQ<true>(bFilters[idx].getQ());
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
        soloFilter.setFreq(freq);
        soloFilter.setQ(q);

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
                switch (filterLRs[i].load()) {
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
                break;
            }
            case filterStructure::matched: {
                const auto singleLatency = prototypeCorrections[0].getLatency();
                newLatency = singleLatency
                             + static_cast<int>(useLR) * singleLatency
                             + static_cast<int>(useMS) * singleLatency;
                break;
            }
            case filterStructure::mixed:
            case filterStructure::linear: {
                break;
            }
        }
        if (newLatency += latency.load()) {
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
                if (dynRelatives[indices[i]].load()) {
                    useTrackers[idx] = true;
                    break;
                }
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
        switch (currentFilterStructure) {
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
        for (size_t idx = 0; idx < bandNUM; ++idx) {
            filters[idx].getMainFilter().template setGain<false>(bFilters[idx].getGain());
            filters[idx].getMainFilter().template setQ<true>(bFilters[idx].getQ());
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
        for (auto &c: prototypeCorrections) {
            c.setToUpdate();
        }
    }

    template
    class Controller<float>;

    template
    class Controller<double>;
}
