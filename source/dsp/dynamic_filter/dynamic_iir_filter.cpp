// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "dynamic_iir_filter.hpp"

namespace zlDynamicFilter {
    template<typename FloatType>
    void IIRFilter<FloatType>::reset() {
        mFilter.reset();
        tFilter.reset();
        sFilter.reset();
        compressor.reset();
    }

    template<typename FloatType>
    void IIRFilter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        mFilter.prepare(spec);
        bFilter.prepare(spec);
        tFilter.prepare(spec);
        sFilter.prepare(spec);
        sFilter.setFilterType(zlIIR::FilterType::bandPass);
        compressor.prepare(spec);
        sBufferCopy.setSize(static_cast<int>(spec.numChannels),
                            static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    void IIRFilter<FloatType>::process(juce::AudioBuffer<FloatType> &mBuffer, juce::AudioBuffer<FloatType> &sBuffer) {
        if (!bypass.load()) {
            if (dynamicON.load()) {
                sBufferCopy.makeCopyOf(sBuffer, true);
                sFilter.process(sBufferCopy);
                auto portion = compressor.process(sBufferCopy);
                if (dynamicBypass.load()) {
                    portion = 1;
                }
//                mainPortion.store(portion);
                mFilter.setGain(portion * bFilter.getGain() + (1 - portion) * tFilter.getGain(), false);
                mFilter.setQ(portion * bFilter.getQ() + (1 - portion) * tFilter.getQ(), true);
            }
            mFilter.process(mBuffer);
        }
    }

    template<typename FloatType>
    FloatType IIRFilter<FloatType>::getSideDefaultFreq() {
        auto mFilterType = bFilter.getFilterType();
        if (mFilterType == zlIIR::FilterType::lowShelf || mFilterType == zlIIR::FilterType::lowPass) {
            return FloatType(0.5) * (FloatType(10) + bFilter.getFreq());
        } else if (mFilterType == zlIIR::FilterType::highShelf || mFilterType == zlIIR::FilterType::highPass) {
            return FloatType(0.5) * (FloatType(20000) + bFilter.getFreq());
        } else {
            return bFilter.getFreq();
        }
    }

    template
    class IIRFilter<float>;

    template
    class IIRFilter<double>;
}