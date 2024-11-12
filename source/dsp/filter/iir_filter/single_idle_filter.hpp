// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLFILTER_IDLEIIR_HPP
#define ZLFILTER_IDLEIIR_HPP

#include "../filter_design/filter_design.hpp"
#include "coeff/martin_coeff.hpp"
#include "iir_base.hpp"
#include "svf_base.hpp"

namespace zlFilter {
    /**
     * an idle IIR filter which holds coeffs for calculating responses
     * @tparam FloatType the float type of input audio buffer
     * @tparam FilterSize the number of cascading filters
     */
    template<typename FloatType, size_t FilterSize>
    class IIRIdle {
    public:
        IIRIdle() = default;

        void prepare(const double sampleRate) {
            fs.store(sampleRate);
            toUpdatePara.store(true);
        }

        void setFreq(const FloatType x) {
            freq.store(x);
            toUpdatePara.store(true);
        }

        FloatType getFreq() const { return static_cast<FloatType>(freq.load()); }

        void setGain(const FloatType x) {
            gain.store(x);
            toUpdatePara.store(true);
        }

        FloatType getGain() const { return static_cast<FloatType>(gain.load()); }

        void setQ(const FloatType x) {
            q.store(x);
            toUpdatePara.store(true);
        }

        void setFilterType(const FilterType x) {
            filterType.store(x);
            toUpdatePara.store(true);
        }

        FilterType getFilterType() const { return filterType.load(); }

        void setOrder(const size_t x) {
            order.store(x);
            toUpdatePara.store(true);
        }

        void prepareResponseSize(const size_t x) {
            response.resize(x);
            std::fill(response.begin(), response.end(), std::complex(FloatType(1), FloatType(0)));
        }

        bool updateResponse(const std::vector<std::complex<FloatType>> &wis) {
            if (toUpdatePara.exchange(false)) {
                updateParas();
                std::fill(response.begin(), response.end(), std::complex(FloatType(1), FloatType(0)));
                for (size_t i = 0; i < currentFilterNum; ++i) {
                    IIRBase<FloatType>::updateResponse(coeffs[i], wis, response);
                }
                return true;
            }
            return false;
        }

        std::vector<std::complex<FloatType> > &getResponse() { return response; }

        void setToUpdate() {toUpdatePara.store(true);}

    private:
        std::array<std::array<double, 6>, FilterSize> coeffs{};
        std::atomic<bool> toUpdatePara{true};
        std::atomic<size_t> order{2};
        size_t currentFilterNum{1};
        std::atomic<double> freq{1000.0}, gain{0.0}, q{0.707};
        std::atomic<double> fs{48000.0};
        std::atomic<FilterType> filterType = FilterType::peak;
        std::vector<std::complex<FloatType> > response{};

        void updateParas() {
            currentFilterNum = updateIIRCoeffs(filterType.load(), order.load(),
                                               freq.load(), fs.load(),
                                               gain.load(), q.load(), coeffs);
        }

        static size_t updateIIRCoeffs(const FilterType filterType, const size_t n,
                                      const double f, const double fs, const double g0, const double q0,
                                      std::array<std::array<double, 6>, FilterSize> &coeffs) {
            return FilterDesign::updateCoeffs<FilterSize,
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

#endif //ZLFILTER_IDLEIIR_HPP
