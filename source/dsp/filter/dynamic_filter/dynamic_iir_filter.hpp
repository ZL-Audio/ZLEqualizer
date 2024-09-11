// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLFILTER_DYNAMIC_IIR_FILTER_HPP
#define ZLFILTER_DYNAMIC_IIR_FILTER_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "../iir_filter/iir_filter.hpp"
#include "../ideal_filter/empty_filter.hpp"
#include "../../compressor/compressor.hpp"

namespace zlFilter {
    /**
     * a dynamic IIR filter which holds a main filter, a base filter, a target filter and a side filter
     * the output signal is filtered by the main filter, whose gain and Q is set by the mix of base/target filters'
     * the mix portion is controlled by a compressor on the signal from the side filter (on the side chain)
     * @tparam FloatType
     */
    template<typename FloatType>
    class DynamicIIR {
    public:
        DynamicIIR() = default;

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * process the audio buffer
         * @param mBuffer main chain audio buffer
         * @param sBuffer side chain audio buffer
         */
        void process(juce::AudioBuffer<FloatType> &mBuffer, juce::AudioBuffer<FloatType> &sBuffer);

        void processBypass();

        IIR<FloatType> &getMainFilter() { return mFilter; }

        Empty<FloatType> &getBaseFilter() { return bFilter; }

        Empty<FloatType> &getTargetFilter() { return tFilter; }

        IIR<FloatType> &getSideFilter() { return sFilter; }

        inline zlCompressor::ForwardCompressor<FloatType> &getCompressor() { return compressor; }

        inline void setBypass(const bool x) { bypass.store(x); }

        inline bool getBypass() const { return bypass.load(); }

        inline void setActive(const bool x) {
            if (x) {
                mFilter.setToRest();
            }
            active.store(x);
        }

        inline void setDynamicON(const bool x) { dynamicON.store(x); }

        inline bool getDynamicON() const { return dynamicON.load(); }

        inline void setDynamicBypass(const bool x) { dynamicBypass.store(x); }

        inline bool getDynamicBypass() const { return dynamicBypass.load(); }

        inline void setCompoensationON(const bool x) { compensation.enable(x); }

        void setSVFON(const bool f) {
            mFilter.setSVFON(f);
            sFilter.setSVFON(f);
        }

        void setIsPerSample(const bool x) { isPerSample.store(x); }

        FloatType getSGC() const {
            if (!active.load() || bypass.load()) {
                return FloatType(0);
            } else {
                return compensation.getGainDecibels();
            }
        }

    private:
        zlFilter::IIR<FloatType> mFilter, sFilter;
        zlFilter::Empty<FloatType> bFilter, tFilter;
        zlFilter::StaticGainCompensation<FloatType> compensation{bFilter};
        zlCompressor::ForwardCompressor<FloatType> compressor;
        juce::AudioBuffer<FloatType> sBufferCopy;
        std::atomic<bool> bypass{true}, active{false}, dynamicON{false}, dynamicBypass{false};
        juce::AudioBuffer<FloatType> sampleBuffer;
        std::atomic<bool> isPerSample{false};
    };
}


#endif //ZLFILTER_DYNAMIC_IIR_FILTER_HPP
