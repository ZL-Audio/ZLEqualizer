// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "dynamic_iir_filter.hpp"

namespace zlFilter {
    template<typename FloatType>
    void DynamicIIR<FloatType>::reset() {
        mFilter.reset();
        tFilter.reset();
        sFilter.reset();
        compressor.reset();
    }

    template<typename FloatType>
    void DynamicIIR<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        mFilter.prepare(spec);
        bFilter.prepare(spec);
        tFilter.prepare(spec);
        sFilter.setOrder(2, false);
        sFilter.setFilterType(zlFilter::FilterType::bandPass, false);
        sFilter.prepare(spec);
        compressor.prepare(spec);
        compensation.prepare(spec);

        compressor.getComputer().setRatio(100);
        sBufferCopy.setSize(static_cast<int>(spec.numChannels),
                            static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    void DynamicIIR<FloatType>::process(juce::AudioBuffer<FloatType> &mBuffer, juce::AudioBuffer<FloatType> &sBuffer) {
        if (!active.load()) { return; }
        if (bFilter.updateParasForDBOnly()) {
            compensation.update();
        }
        tFilter.updateParasForDBOnly();
        sFilter.updateParas();
        const auto currentBypass = bypass.load();
        if (dynamicON.load()) {
            sBufferCopy.makeCopyOf(sBuffer, true);
            sFilter.process(sBufferCopy);
            auto reducedLoudness = juce::Decibels::gainToDecibels(compressor.process(sBufferCopy));
            auto maximumReduction = compressor.getComputer().getReductionAtKnee();
            auto portion = std::min(reducedLoudness / maximumReduction, FloatType(1));
            if (dynamicBypass.load()) {
                portion = 0;
            }
            if (!isPerSample.load()) {
                mFilter.setGain((1 - portion) * bFilter.getGain() + portion * tFilter.getGain(), false);
                mFilter.setQ((1 - portion) * bFilter.getQ() + portion * tFilter.getQ(), true);

                mFilter.process(mBuffer, currentBypass);
            } else {
                const auto oldGain = mFilter.getGain();
                const auto oldQ = mFilter.getQ();
                const auto newGain = (1 - portion) * bFilter.getGain() + portion * tFilter.getGain();
                const auto newQ = (1 - portion) * bFilter.getQ() + portion * tFilter.getQ();
                auto audioWriters = mBuffer.getArrayOfWritePointers();
                for (int i = 0; i < mBuffer.getNumSamples(); ++i) {
                    const auto pp = static_cast<FloatType>(i) / static_cast<FloatType>(mBuffer.getNumSamples());
                    const auto currentGain = newGain * pp + oldGain * (1 - pp);
                    const auto currentQ = newQ * pp + oldQ * (1 - pp);
                    mFilter.setGain(currentGain, false);
                    mFilter.setQ(currentQ, true);
                    sampleBuffer.setDataToReferTo(audioWriters, static_cast<int>(mBuffer.getNumChannels()), i, 1);
                    mFilter.process(sampleBuffer, currentBypass);
                }
            }
        } else {
            mFilter.process(mBuffer, currentBypass);
        }
        if (!currentBypass) {
            compensation.process(mBuffer);
        }
    }

    template<typename FloatType>
    void DynamicIIR<FloatType>::processBypass() {
        if (bFilter.updateParas()) {
            compensation.update();
        }
        mFilter.updateParas();
        tFilter.updateParas();
        sFilter.updateParas();
    }

    template
    class DynamicIIR<float>;

    template
    class DynamicIIR<double>;
}
