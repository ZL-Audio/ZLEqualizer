// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "controller.h"

namespace zlDSP {
    template<typename FloatType>
    Controller<FloatType>::Controller(juce::AudioProcessor &processor) :processorRef(processor) {}

    template<typename FloatType>
    void Controller<FloatType>::reset() {
        for (auto &f: filters) {
            f.reset();
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        subBuffer.prepare(spec);
        subBuffer.setSubBufferSize(static_cast<int>(subBufferLength * spec.sampleRate));
        processorRef.setLatencySamples(static_cast<int>(subBuffer.getLatencySamples()));
        juce::dsp::ProcessSpec subSpec{spec.sampleRate, subBuffer.getSubSpec().maximumBlockSize, 2};
        for (auto &f: filters) {
            f.prepare(subSpec);
        }
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
                if (filterLRs[i].load() == lrTypes::stereo) {
                    filters[i].process(subMainBuffer, subSideBuffer);
                }
            }
            // LR filters process
            if (useLR.load()) {
                lrMainSplitter.split(subMainBuffer);
                if (useDynamic.load()) {
                    lrSideSplitter.split(subSideBuffer);
                }
                for (size_t i = 0; i < bandNUM; ++i) {
                    if (filterLRs[i].load() == lrTypes::left) {
                        filters[i].process(lrMainSplitter.getLBuffer(), lrSideSplitter.getLBuffer());
                    } else if (filterLRs[i].load() == lrTypes::right) {
                        filters[i].process(lrMainSplitter.getRBuffer(), lrSideSplitter.getRBuffer());
                    }
                }
                lrMainSplitter.combine(subMainBuffer);
            }
            // MS filters process
            if (useMS.load()) {
                msMainSplitter.split(subMainBuffer);
                if (useDynamic.load()) {
                    msSideSplitter.split(subSideBuffer);
                }
                for (size_t i = 0; i < bandNUM; ++i) {
                    if (filterLRs[i].load() == lrTypes::mid) {
                        filters[i].process(msMainSplitter.getMBuffer(), msSideSplitter.getMBuffer());
                    } else if (filterLRs[i].load() == lrTypes::side) {
                        filters[i].process(msMainSplitter.getSBuffer(), msSideSplitter.getSBuffer());
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
    void Controller<FloatType>::setFilterLRs(zlDSP::lrTypes x, size_t idx) {
        // prepare the filter
        filterLRs[idx].store(x);
        if (x == lrTypes::stereo) {
            filters[idx].prepare({subBuffer.getSubSpec().sampleRate, subBuffer.getSubSpec().maximumBlockSize, 2});
        } else {
            filters[idx].prepare({subBuffer.getSubSpec().sampleRate, subBuffer.getSubSpec().maximumBlockSize, 1});
        }
        // update useLR and useMS
        useLR.store(false);
        for (auto &lr: filterLRs) {
            if (lr.load() == lrTypes::left || lr.load() == lrTypes::right) {
                useLR.store(true);
                break;
            }
        }
        useMS.store(false);
        for (auto &lr: filterLRs) {
            if (lr.load() == lrTypes::mid || lr.load() == lrTypes::side) {
                useMS.store(true);
                break;
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::setDynamicON(bool x, size_t idx) {
        filters[idx].setDynamicON(x);
        useDynamic.store(false);
        for (auto &f: filters) {
            if (f.getDynamicON()) {
                useDynamic.store(true);
                break;
            }
        }
    }

    template
    class Controller<float>;

    template
    class Controller<double>;
}