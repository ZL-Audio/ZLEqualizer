// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_SINGLE_FILTER_HPP
#define ZLEQUALIZER_SINGLE_FILTER_HPP

#include <juce_dsp/juce_dsp.h>
#include "../filter_design/filter_design.hpp"
#include "coeff/martin_coeff.hpp"
#include "iir_base.hpp"
#include "svf_base.hpp"

namespace zlFilter {
    /**
     * an IIR filter which processes audio on the real-time thread
     * the maximum modulation rate of parameters is once per block
     * @tparam FloatType
     */
    template<typename FloatType>
    class IIR {
    public:
        IIR() = default;

        void reset();

        void setToRest() { toReset.store(true); }

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * prepare for processing the incoming audio buffer
         * call it when you want to update filter parameters
         * @param buffer
         */
        void processPre(juce::AudioBuffer<FloatType> &buffer);

        /**
         * process the incoming audio buffer
         * for parallel filter, call it with the internal parallel buffer
         * @param buffer
         * @param isBypassed
         */
        void process(juce::AudioBuffer<FloatType> &buffer, bool isBypassed = false);

        /**
         * add the processed parallel buffer to the incoming audio buffer
         * @param buffer
         * @param isBypassed
         */
        void processParallelPost(juce::AudioBuffer<FloatType> &buffer, bool isBypassed = false);

        /**
         * set the frequency of the filter
         * if frequency changes >= 2 octaves, the filter will reset
         * @param x frequency
         * @param update whether update filter coefficient
         */
        void setFreq(FloatType x, bool update = true);

        inline FloatType getFreq() const { return static_cast<FloatType>(freq.load()); }

        /**
         * set the gain of the filter
         * @param x gain
         * @param update whether update filter coefficient
         */
        void setGain(FloatType x, bool update = true);

        inline FloatType getGain() const { return static_cast<FloatType>(gain.load()); }

         void setGainNow(FloatType x);

        /**
         * set the Q value of the filter
         * @param x Q value
         * @param update whether update filter coefficient
         */
        void setQ(FloatType x, bool update = true);

        inline FloatType getQ() const { return static_cast<FloatType>(q.load()); }

        void setGainAndQNow(FloatType g1, FloatType q1);

        /**
         * set the type of the filter, the filter will always reset
         * @param x filter type
         * @param update whether update filter coefficient
         */
        void setFilterType(FilterType x, bool update = true);

        inline FilterType getFilterType() const { return filterType.load(); }

        /**
         * set the order of the filter, the filter will always reset
         * @param x filter order
         * @param update whether update filter coefficient
         */
        void setOrder(size_t x, bool update = true);

        inline size_t getOrder() const { return order.load(); }

        /**
         * update filter coefficients
         * DO NOT call it unless you are sure what you are doing
         */
        void updateCoeffs();

        /**
         * get the number of 2nd order filters
         * @return
         */
        size_t getFilterNum() const { return filterNum.load(); }

        /**
         * get the num of channels
         * @return
         */
        inline juce::uint32 getNumChannels() const { return numChannels.load(); }

        /**
         * get the array of 2nd order filters
         * @return
         */
        std::array<IIRBase<FloatType>, 16> &getFilters() { return filters; }

        void setFilterStructure(const FilterStructure x) {
            filterStructure.store(x);
        }

        bool getShouldBeParallel() const { return shouldBeParallel; }

        bool getShouldNotBeParallel() const { return shouldNotBeParallel; }


        juce::AudioBuffer<FloatType> &getParallelBuffer() { return parallelBuffer; }

    private:
        std::array<IIRBase<FloatType>, 16> filters{};
        juce::AudioBuffer<FloatType> parallelBuffer;

        std::atomic<size_t> filterNum{1};
        size_t currentFilterNum{1};
        std::atomic<double> freq = 1000, gain = 0, q = 0.707;
        std::atomic<size_t> order{2};
        std::atomic<FilterType> filterType{FilterType::peak};
        FilterType currentFilterType{FilterType::peak};
        bool bypassNextBlock{false};

        juce::dsp::ProcessSpec processSpec{48000, 512, 2};
        std::atomic<float> sampleRate{48000};
        std::atomic<juce::uint32> numChannels;

        std::atomic<bool> toUpdatePara = false, toReset = false;

        std::array<std::array<double, 6>, 16> coeffs{};

        std::atomic<bool> useSVF{false};
        bool currentUseSVF{false};
        std::array<SVFBase<FloatType>, 16> svfFilters{};

        std::atomic<FilterStructure> filterStructure{FilterStructure::iir};
        FilterStructure currentFilterStructure{FilterStructure::iir};
        bool shouldBeParallel{false}, shouldNotBeParallel{false};
        FloatType parallelMultiplier;

        static size_t updateIIRCoeffs(const FilterType filterType, const size_t n,
                                      const double f, const double fs, const double g0, const double q0,
                                      std::array<std::array<double, 6>, 16> &coeffs) {
            return FilterDesign::updateCoeffs<16,
                MartinCoeff::get1LowShelf, MartinCoeff::get1HighShelf, MartinCoeff::get1TiltShelf,
                MartinCoeff::get1LowPass, MartinCoeff::get1HighPass,
                MartinCoeff::get2Peak,
                MartinCoeff::get2LowShelf, MartinCoeff::get2HighShelf, MartinCoeff::get2TiltShelf,
                MartinCoeff::get2LowPass, MartinCoeff::get2HighPass,
                MartinCoeff::get2BandPass, MartinCoeff::get2Notch>(
                filterType, n, f, fs, g0, q0, coeffs);
        }

        void updateParallelGain(double x) {
            parallelMultiplier = juce::Decibels::decibelsToGain<FloatType>(static_cast<FloatType>(x)) - FloatType(1);
        }
    };
}

#endif //ZLEQUALIZER_SINGLE_FILTER_HPP
