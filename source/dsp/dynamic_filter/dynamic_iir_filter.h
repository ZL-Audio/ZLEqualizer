// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_DYNAMIC_IIR_FILTER_H
#define ZLEQUALIZER_DYNAMIC_IIR_FILTER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "../iir_filter/iir_filter.h"
#include "../compressor/compressor.h"

namespace zlDynamicFilter {
    /**
     * a dynamic IIR filter which holds a main filter, a base filter, a target filter and a side filter
     * the output signal is filtered by the main filter, whose gain and Q is set by the mix of base/target filters'
     * the mix portion is controlled by a compressor on the signal from the side filter (on the side chain)
     * @tparam FloatType
     */
    template<typename FloatType>
    class IIRFilter {
    public:
        IIRFilter() = default;

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * process the audio buffer
         * @param mBuffer main chain audio buffer
         * @param sBuffer side chain audio buffer
         */
        void process(juce::AudioBuffer<FloatType> &mBuffer, juce::AudioBuffer<FloatType> &sBuffer);

        inline zlIIR::Filter<FloatType> &getMainFilter() { return mFilter; }

        inline zlIIR::Filter<FloatType> &getBaseFilter() { return bFilter; }

        inline zlIIR::Filter<FloatType> &getTargetFilter() { return tFilter; }

        inline zlIIR::Filter<FloatType> &getSideFilter() { return sFilter; }

        inline zlCompressor::ForwardCompressor<FloatType> &getCompressor() { return compressor; }

        inline void setBypass(bool x) { bypass.store(x); }

        inline bool getBypass() { return bypass.load(); }

        inline void setDynamicON(bool x) { dynamicON.store(x); }

        inline bool getDynamicON() { return dynamicON.load(); }

        inline void setDynamicBypass(bool x) { dynamicBypass.store(x); }

        inline bool getDynamicBypass() { return dynamicBypass.load(); }

        /**
         * calculate the default frequency of the side filter
         * @return the default frequency of the side filter
         */
        FloatType getSideDefaultFreq();

    private:
        zlIIR::Filter<FloatType> mFilter, bFilter, tFilter, sFilter;
        zlCompressor::ForwardCompressor<FloatType> compressor;
        juce::AudioBuffer<FloatType> sBufferCopy;
        std::atomic<bool> bypass = true, dynamicON = false, dynamicBypass = false;

//        std::atomic<FloatType> mainPortion;

//        juce::FileLogger logger{juce::File("/Volumes/Ramdisk/log.txt"), "Dynamic IIR Log"};
    };
}


#endif //ZLEQUALIZER_DYNAMIC_IIR_FILTER_H
