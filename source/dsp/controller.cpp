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
    Controller<FloatType>::Controller(juce::AudioProcessor &processor) : processorRef(processor) {
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
        subBuffer.prepare({spec.sampleRate, spec.maximumBlockSize, 4});
        subBuffer.setSubBufferSize(static_cast<int>(subBufferLength * spec.sampleRate));
        latencyInSamples.store(static_cast<int>(subBuffer.getLatencySamples()));
        triggerAsyncUpdate();
        juce::dsp::ProcessSpec subSpec{spec.sampleRate, subBuffer.getSubSpec().maximumBlockSize, 2};
        for (auto &f: filters) {
            f.prepare(subSpec);
        }
        soloFilter.prepare(subSpec);
        lrMainSplitter.prepare(subSpec);
        lrSideSplitter.prepare(subSpec);
        msMainSplitter.prepare(subSpec);
        msSideSplitter.prepare(subSpec);
    }

    template<typename FloatType>
    void Controller<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        juce::AudioBuffer<FloatType> mainBuffer(processorRef.getBusBuffer(buffer, true, 0));
        juce::AudioBuffer<FloatType> sideBuffer(processorRef.getBusBuffer(buffer, true, 1));
        // if no side chain, copy the main buffer into the side buffer
        if (!sideChain.load() && useDynamic.load()) {
            sideBuffer.makeCopyOf(mainBuffer, true);
        }
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        const juce::ScopedReadLock scopedLock(paraUpdateLock);
        // ---------------- start sub buffer
        subBuffer.pushBlock(block);
        while (subBuffer.isSubReady()) {
            subBuffer.popSubBuffer();
            // create main sub buffer and side sub buffer
            auto subMainBuffer = juce::AudioBuffer<FloatType>(subBuffer.subBuffer.getArrayOfWritePointers() + 0,
                                                              2, subBuffer.subBuffer.getNumSamples());
            auto subSideBuffer = juce::AudioBuffer<FloatType>(subBuffer.subBuffer.getArrayOfWritePointers() + 2,
                                                              2, subBuffer.subBuffer.getNumSamples());
            // stereo filters process
            for (size_t i = 0; i < bandNUM; ++i) {
                if (filterLRs[i].load() == lrType::stereo) {
                    processOneFilter(i, subMainBuffer, subSideBuffer);
                }
            }
            // LR filters process
            if (useLR.load()) {
                lrMainSplitter.split(subMainBuffer);
                if (useDynamic.load()) {
                    lrSideSplitter.split(subSideBuffer);
                }
                for (size_t i = 0; i < bandNUM; ++i) {
                    if (filterLRs[i].load() == lrType::left) {
                        processOneFilter(i, lrMainSplitter.getLBuffer(), lrSideSplitter.getLBuffer());
                        if (useSolo.load() && soloIdx.load() == i) { lrMainSplitter.getRBuffer().applyGain(0); }
                    } else if (filterLRs[i].load() == lrType::right) {
                        processOneFilter(i, lrMainSplitter.getRBuffer(), lrSideSplitter.getRBuffer());
                        if (useSolo.load() && soloIdx.load() == i) { lrMainSplitter.getLBuffer().applyGain(0); }
                    }
                }
                lrMainSplitter.combine(subMainBuffer);
            }
            // MS filters process
            if (useMS.load()) {
                msMainSplitter.split(subMainBuffer);
                for (size_t i = 0; i < bandNUM; ++i) {
                    if (filterLRs[i].load() == lrType::mid) {
                        processOneFilter(i, msMainSplitter.getMBuffer(), msSideSplitter.getMBuffer());
                        if (useSolo.load() && soloIdx.load() == i) { msMainSplitter.getSBuffer().applyGain(0); }
                    } else if (filterLRs[i].load() == lrType::side) {
                        processOneFilter(i, msMainSplitter.getSBuffer(), msSideSplitter.getMBuffer());
                        if (useSolo.load() && soloIdx.load() == i) { msMainSplitter.getMBuffer().applyGain(0); }
                    }
                }
                msMainSplitter.combine(subMainBuffer);
            }
            subBuffer.pushSubBuffer();
        }
        subBuffer.popBlock(block);
        // ---------------- end sub buffer
    }

    template<typename FloatType>
    void Controller<FloatType>::processOneFilter(size_t i,
                                                 juce::AudioBuffer<FloatType> &mainBuffer,
                                                 juce::AudioBuffer<FloatType> &sideBuffer) {
        if (!useSolo.load()) {
            filters[i].process(mainBuffer, sideBuffer);
        } else if (useSolo.load() && soloIdx.load() == i) {
            if (soloSide.load()) {
                if (useDynamic.load()) {
                    mainBuffer.makeCopyOf(sideBuffer);
                } else {
                    mainBuffer.applyGain(0);
                }
            }
            soloFilter.process(mainBuffer);
        }
    }


    template<typename FloatType>
    void Controller<FloatType>::setFilterLRs(const zlDSP::lrType::lrTypes x,
                                             const size_t idx) {
        const juce::ScopedWriteLock scopedLock(paraUpdateLock);
        // prepare the filter
        filterLRs[idx].store(x);
        if (x == lrType::stereo) {
            filters[idx].prepare({subBuffer.getSubSpec().sampleRate, subBuffer.getSubSpec().maximumBlockSize, 2});
        } else {
            filters[idx].prepare({subBuffer.getSubSpec().sampleRate, subBuffer.getSubSpec().maximumBlockSize, 1});
        }
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
    }

    template<typename FloatType>
    void Controller<FloatType>::setDynamicON(const bool x, size_t idx) {
        const juce::ScopedWriteLock scopedLock(paraUpdateLock);
        filters[idx].setDynamicON(x);
        filters[idx].getMainFilter().setGain(filters[idx].getBaseFilter().getGain(), false);
        filters[idx].getMainFilter().setQ(filters[idx].getBaseFilter().getQ(), true);
        useDynamic.store(false);
        for (auto &f: filters) {
            if (f.getDynamicON()) {
                useDynamic.store(true);
                break;
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::addDBs(std::array<FloatType, zlIIR::frequencies.size()> &x) {
        const juce::ScopedReadLock scopedLock(magLock);
        std::transform(x.begin(), x.end(), dBs.begin(), x.begin(), std::plus<FloatType>());
    }

    template<typename FloatType>
    void Controller<FloatType>::updateDBs() {
        const juce::ScopedWriteLock scopedLock(magLock);
        dBs.fill(FloatType(0));
        for (size_t i = 0; i < bandNUM; i++) {
            if (filterLRs[i].load() == lrType::stereo) {
                const juce::ScopedReadLock localScopedLock(filters[i].getMainFilter().getMagLock());
                std::transform(dBs.begin(), dBs.end(),
                               filters[i].getMainFilter().getDBs().begin(),
                               dBs.begin(), std::plus<FloatType>());
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::handleAsyncUpdate() {
        processorRef.setLatencySamples(latencyInSamples.load());
    }

    template<typename FloatType>
    std::tuple<FloatType, FloatType> Controller<FloatType>::getSoloFilterParas(zlIIR::Filter<FloatType> &baseFilter) {
        switch (baseFilter.getFilterType()) {
            case zlIIR::FilterType::lowPass:
            case zlIIR::FilterType::lowShelf: {
                auto soloFreq = static_cast<FloatType>(std::sqrt(1) * std::sqrt(baseFilter.getFreq()));
                auto scale = soloFreq;
                soloFreq = static_cast<FloatType>(std::min(std::max(soloFreq, FloatType(10)), FloatType(20000)));
                auto bw = std::max(std::log2(scale) * 2, FloatType(0.01));
                auto soloQ = 1 / (2 * std::sinh(std::log(FloatType(2)) / 2 * bw));
                soloQ = std::min(std::max(soloQ, FloatType(0.025)), FloatType(25));
                return {soloFreq, soloQ};
            }
            case zlIIR::FilterType::highPass:
            case zlIIR::FilterType::highShelf: {
                auto soloFreq = static_cast<FloatType>(std::sqrt(20000) * std::sqrt(baseFilter.getFreq()));
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
        soloFilter.setFilterType(zlIIR::FilterType::bandPass, false);
        FloatType freq, q;
        if (!isSide) {
            std::tie(freq, q) = getSoloFilterParas(filters[idx].getMainFilter());
        } else {
            std::tie(freq, q) = getSoloFilterParas(filters[idx].getSideFilter());
        }
        soloFilter.setFreq(freq, false);
        soloFilter.setQ(q, false);
        const juce::ScopedWriteLock scopedLock(paraUpdateLock);
        useSolo.store(true);
        // if (filterLRs[idx] == lrType::stereo) {
        soloFilter.prepare({subBuffer.getSubSpec().sampleRate, subBuffer.getSubSpec().maximumBlockSize, 2});
        // } else {
        //     soloFilter.prepare({subBuffer.getSubSpec().sampleRate, subBuffer.getSubSpec().maximumBlockSize, 1});
        // }
        soloIdx.store(idx);
        soloSide.store(isSide);
    }

    template
    class Controller<float>;

    template
    class Controller<double>;
}
