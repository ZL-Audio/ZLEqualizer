// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLFILTER_DYNAMIC_IIR_FILTER_HPP
#define ZLFILTER_DYNAMIC_IIR_FILTER_HPP

#include <juce_dsp/juce_dsp.h>

#include "../iir_filter/iir_filter.hpp"
#include "../ideal_filter/ideal_filter.hpp"
#include "../../compressor/compressor.hpp"

namespace zlFilter {
    /**
     * a dynamic IIR filter which holds a main filter, a base filter, a target filter and a side filter
     * the output signal is filtered by the main filter, whose gain and Q is set by the mix of base/target filters'
     * the mix portion is controlled by a compressor on the signal from the side filter (on the side chain)
     * @tparam FloatType
     */
    template<typename FloatType, size_t FilterSize>
    class DynamicIIR {
    public:
        DynamicIIR(zlFilter::Empty<FloatType> &b, zlFilter::Empty<FloatType> &t)
            : bFilter(b), tFilter(t) {
        }

        void reset() {
            mFilter.reset();
            sFilter.reset();
        }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            mFilter.prepare(spec);
            sFilter.template setOrder<false>(2);
            sFilter.template setFilterType<false>(zlFilter::FilterType::bandPass);
            sFilter.prepare(spec);
            juce::dsp::ProcessSpec temp = spec;
            temp.sampleRate = 1000.0;
            follower.prepare(temp);
            tracker.prepare(temp);

            computer.setRatio(1000);
            sBufferCopy.setSize(static_cast<int>(spec.numChannels),
                                static_cast<int>(spec.maximumBlockSize));
        }

        /**
         * process the audio buffer
         * @param mBuffer main chain audio buffer
         * @param sBuffer side chain audio buffer
         */
        template<bool isBypassed = false>
        void process(juce::AudioBuffer<FloatType> &mBuffer, juce::AudioBuffer<FloatType> &sBuffer) {
            cacheCurrentValues();
            mFilter.processPre(mBuffer);
            if (currentDynamicON) {
                if (!mFilter.getShouldNotBeParallel()) {
                    processDynamic<isBypassed>(mBuffer, sBuffer);
                }
            } else {
                if (mFilter.getShouldBeParallel()) {
                    mFilter.template process<isBypassed>(mFilter.getParallelBuffer());
                } else if (!mFilter.getShouldNotBeParallel()) {
                    mFilter.template process<isBypassed>(mBuffer);
                }
            }
        }

        template<bool isBypassed = false>
        void processParallelPost(juce::AudioBuffer<FloatType> &mBuffer, juce::AudioBuffer<FloatType> &sBuffer) {
            if (mFilter.getShouldNotBeParallel()) {
                if (currentDynamicON) {
                    processDynamic<isBypassed>(mBuffer, sBuffer);
                } else {
                    mFilter.template process<isBypassed>(mBuffer);
                }
            } else if (mFilter.getShouldBeParallel()) {
                mFilter.template processParallelPost<isBypassed>(mBuffer);
            }
        }

        static void processBypass() {
        }

        IIR<FloatType, FilterSize> &getMainFilter() { return mFilter; }

        IIR<FloatType, FilterSize> &getSideFilter() { return sFilter; }

        zlCompressor::KneeComputer<FloatType, false, false> &getComputer() { return computer; }

        zlCompressor::RMSTracker<FloatType, false> &getTracker() { return tracker; }

        zlCompressor::PSFollower<FloatType, true, false> &getFollower() { return follower; }

        void setActive(const bool x) {
            if (x) {
                mFilter.setToRest();
            }
            active.store(x);
        }

        void setDynamicON(const bool x) { dynamicON.store(x); }

        bool getDynamicON() const { return dynamicON.load(); }

        void setDynamicBypass(const bool x) { dynamicBypass.store(x); }

        bool getDynamicBypass() const { return dynamicBypass.load(); }

        void setFilterStructure(const FilterStructure x) {
            filterStructure.store(x);
        }

        void setIsPerSample(const bool x) { isPerSample.store(x); }

        void setBaseLine(const FloatType x) { compBaseline = x; }

        FloatType getBaseLine() const { return compBaseline; }

    private:
        zlFilter::IIR<FloatType, FilterSize> mFilter, sFilter;
        zlFilter::Empty<FloatType> &bFilter, &tFilter;
        zlCompressor::KneeComputer<FloatType, false, false> computer;
        zlCompressor::RMSTracker<FloatType, false> tracker;
        zlCompressor::PSFollower<FloatType, true, false> follower;
        FloatType compBaseline;
        juce::AudioBuffer<FloatType> sBufferCopy;
        std::atomic<bool> active{false}, dynamicON{false}, dynamicBypass{false};
        bool currentDynamicON{false}, currentDynamicBypass{false};
        std::atomic<FilterStructure> filterStructure{FilterStructure::iir};
        FilterStructure currentFilterStructure{FilterStructure::iir};
        juce::AudioBuffer<FloatType> sampleBuffer;
        std::atomic<bool> isPerSample{false};
        bool currentIsPerSample{false};

        template<bool isBypassed = false>
        void processDynamic(juce::AudioBuffer<FloatType> &mBuffer, juce::AudioBuffer<FloatType> &sBuffer) {
            sBufferCopy.makeCopyOf(sBuffer, true);
            sFilter.processPre(sBufferCopy);
            sFilter.process(sBufferCopy);
            // feed side-chain into the tracker
            tracker.processBufferRMS(sBufferCopy);
            // get loudness from tracker
            const auto currentLoudness = tracker.getMomentaryLoudness() - compBaseline;
            // calculate the reduction
            const auto reducedLoudness = currentLoudness - computer.eval(currentLoudness);
            // smooth the reduction
            const auto smoothLoudness = follower.processSample(reducedLoudness);

            // calculate gain/Q mix portion
            auto portion = std::min(smoothLoudness / computer.getReductionAtKnee(), FloatType(1));

            // DBG(juce::String(currentLoudness) + "\t" + juce::String(reducedLoudness) + "\t" +
            //     juce::String(smoothLoudness) + "\t" + juce::String(portion));
            if (currentDynamicBypass) {
                portion = 0;
            }
            if (currentIsPerSample) {
                mFilter.template setGain<true, false, false>(
                    (1 - portion) * bFilter.getGain() + portion * tFilter.getGain());
                mFilter.template setQ<true, false, false>((1 - portion) * bFilter.getQ() + portion * tFilter.getQ());
            } else {
                mFilter.template setGain<true, false, true>(
                    (1 - portion) * bFilter.getGain() + portion * tFilter.getGain());
                mFilter.template setQ<true, false, true>((1 - portion) * bFilter.getQ() + portion * tFilter.getQ());
                mFilter.updateCoeffs();
            }
            if (mFilter.getShouldBeParallel()) {
                mFilter.template process<isBypassed>(mFilter.getParallelBuffer());
            } else {
                mFilter.template process<isBypassed>(mBuffer);
            }
        }

        void cacheCurrentValues() {
            if (currentFilterStructure != filterStructure.load()) {
                currentFilterStructure = filterStructure.load();
                switch (currentFilterStructure) {
                    case FilterStructure::iir:
                    case FilterStructure::svf: {
                        mFilter.setFilterStructure(currentFilterStructure);
                        sFilter.setFilterStructure(currentFilterStructure);
                        break;
                    }
                    case FilterStructure::parallel: {
                        mFilter.setFilterStructure(currentFilterStructure);
                        sFilter.setFilterStructure(FilterStructure::iir);
                    }
                }
            }
            currentDynamicON = dynamicON.load();
            if (currentDynamicON) {
                currentDynamicBypass = dynamicBypass.load();
                currentIsPerSample = isPerSample.load();
                computer.prepareBuffer();
                tracker.prepareBuffer();
                follower.prepareBuffer();
            }
        }
    };
}


#endif //ZLFILTER_DYNAMIC_IIR_FILTER_HPP
