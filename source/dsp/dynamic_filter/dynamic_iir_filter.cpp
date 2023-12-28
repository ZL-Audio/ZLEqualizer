// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "dynamic_iir_filter.h"

namespace zlDynamicFilter {
    template<typename FloatType>
    void IIRFilter<FloatType>::reset() {
        mFilter.reset();
        tFilter.reset();
        sFilter.reset();
        compressor.reset();
        mixer.reset();
    }

    template<typename FloatType>
    void IIRFilter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        mFilter.prepare(spec);
        tFilter.prepare(spec);
        sFilter.prepare(spec);
        sFilter.setFilterType(zlIIR::FilterType::bandPass);
        compressor.prepare(spec);
        mixer.prepare(spec);
        tBuffer.setSize(static_cast<int>(spec.numChannels),
                        static_cast<int>(spec.maximumBlockSize));
        sBufferCopy.setSize(static_cast<int>(spec.numChannels),
                            static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    void IIRFilter<FloatType>::process(juce::AudioBuffer<FloatType> &mBuffer, juce::AudioBuffer<FloatType> &sBuffer) {
        if (!bypass.load()) {
            mFilter.process(mBuffer);
            if (dynamicON.load()) {
                sBufferCopy.makeCopyOf(sBuffer, true);
                sFilter.process(sBufferCopy);
                dryMixPortion.store(compressor.process(sBufferCopy));
                mixer.pushDrySamples(juce::dsp::AudioBlock<FloatType>(mBuffer));

                tBuffer.makeCopyOf(mBuffer, true);
                tFilter.process(tBuffer);
                if (dynamicBypass.load()) {
                    mixer.setWetMixProportion(1 - dryMixPortion.load());
                    mixer.mixWetSamples(juce::dsp::AudioBlock<FloatType>(tBuffer));
                }
            }
        }
    }

    template<typename FloatType>
    FloatType IIRFilter<FloatType>::getSideDefaultFreq() {
        auto mFilterType = mFilter.getFilterType();
        if (mFilterType == zlIIR::FilterType::lowShelf || mFilterType == zlIIR::FilterType::lowPass) {
            return FloatType(0.5) * (FloatType(10) + mFilter.getFreq());
        } else if (mFilterType == zlIIR::FilterType::highShelf || mFilterType == zlIIR::FilterType::highPass) {
            return FloatType(0.5) * (FloatType(20000) + mFilter.getFreq());
        } else {
            return mFilter.getFreq();
        }
    }

    template<typename FloatType>
    void IIRFilter<FloatType>::addDBs(std::array<FloatType, zlIIR::frequencies.size()> &x) {
        const juce::ScopedReadLock scopedLock(magLock);
        std::transform(x.begin(), x.end(), dBs.begin(), x.begin(), std::plus<FloatType>());
    }

    template<typename FloatType>
    void IIRFilter<FloatType>::updateDBs() {
        const juce::ScopedWriteLock scopedLock(magLock);
        gains.fill(FloatType(0));
        if (!dynamicON.load()) {
            mFilter.addGains(gains);
        } else {
            auto portion = dryMixPortion.load();
            mFilter.addGains(gains, portion);
            tFilter.addGains(gains, 1 - portion);
        }
        std::transform(gains.begin(), gains.end(), dBs.begin(),
                       [](auto &c) { return juce::Decibels::gainToDecibels(c); });
    }

    template
    class IIRFilter<float>;

    template
    class IIRFilter<double>;
}