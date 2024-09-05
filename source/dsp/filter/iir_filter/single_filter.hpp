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
#include "static_frequency_array.hpp"
#include "iir_base.hpp"
#include "svf_base.hpp"
#include "../../farbot/RealtimeObject.hpp"

namespace zlFilter {
    /**
     * a lock free, thread safe static IIR filter
     * it processes audio the the real-time thread, and the response curve can be accessed in another non-realtime thread
     * make sure there is at most one non-realtime thread accessing the response curve data
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

        void process(juce::AudioBuffer<FloatType> &buffer, bool isBypassed = false);

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

        /**
         * set the Q value of the filter
         * @param x Q value
         * @param update whether update filter coefficient
         */
        void setQ(FloatType x, bool update = true);

        inline FloatType getQ() const { return static_cast<FloatType>(q.load()); }

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
         * @return where coefficients have been updated
         */
        bool updateParas();

        /**
         * update filter coefficients for dB calculation
         * DO NOT call it unless you are sure what you are doing
         * @return where coefficients have been updated
         */
        bool updateParasForDBOnly();

        /**
         * get the number of 2nd order filters
         * @return
         */
        size_t getFilterNum() const { return filterNum.load(); }

        /**
         * add response curve (dB) of this filter (multiplied by the scale) to an input array
         * @param x input array
         * @param scale
         */
        void addDBs(std::array<double, frequencies.size()> &x, FloatType scale = 1.0);

        /**
         * add response curve (gain) of this filter (multiplied by the scale) to an input array
         * @param x input array
         * @param scale
         */
        void addGains(std::array<double, frequencies.size()> &x, FloatType scale = 1.0);

        /**
         * return the array of response curve (dB)
         * @return
         */
        inline std::array<double, frequencies.size()> &getDBs() { return dBs; }

        /**
         * get the response (dB) at a specific frequency
         * @param f frequency
         * @return
         */
        FloatType getDB(FloatType f);

        /**
        * return the array of response curve (dB)
        * @return
        */
        inline std::array<double, frequencies.size()> &getGains() { return gains; }

        /**
         * get the num of channels
         * @return
         */
        inline juce::uint32 getNumChannels() const { return numChannels.load(); }

        /**
         * update current response curve
         */
        void updateDBs();

        /**
         * get the array of 2nd order filters
         * @return
         */
        std::array<IIRBase<FloatType>, 16> &getFilters() { return filters; }

        /**
         * get whether the response curve is outdated
         * @return
         */
        inline bool getMagOutdated() const { return magOutdated.load(); }

        /**
         * get whether the response curve is outdated, and set it to x atomically
         * @param x
         * @return
         */
        inline bool getMagOutdated(const bool x) { return magOutdated.exchange(x); }

        /**
         * set the outdated flag to f
         * @param f
         */
        void setMagOutdated(const bool f) { magOutdated.store(f); }

        void setSVFON(const bool f) { useSVF.store(f); }

    private:
        std::array<IIRBase<FloatType>, 16> filters{};

        std::atomic<size_t> filterNum{1};
        std::atomic<double> freq = 1000, gain = 0, q = 0.707;
        std::atomic<size_t> order = 2;
        std::atomic<FilterType> filterType = FilterType::peak;
        juce::dsp::ProcessSpec processSpec{48000, 512, 2};
        std::atomic<float> sampleRate{48000};
        std::atomic<juce::uint32> numChannels;

        std::array<double, frequencies.size()> dBs{}, gains{};
        std::atomic<bool> magOutdated = false;

        std::atomic<bool> toUpdatePara = false, toReset = false;

        std::array<std::array<double, 6>, 16> coeffs{};
        farbot::RealtimeObject<std::array<std::array<double, 6>, 16>, farbot::RealtimeObjectOptions::realtimeMutatable>
        recentCoeffs;

        std::atomic<bool> useSVF{false};
        bool currentUseSVF{false};
        std::array<SVFBase<FloatType>, 16> svfFilters{};
        std::atomic<bool> bypassNextBlock{false};

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
    };
}

#endif //ZLEQUALIZER_SINGLE_FILTER_HPP
