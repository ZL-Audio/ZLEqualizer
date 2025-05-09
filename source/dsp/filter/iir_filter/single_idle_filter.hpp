// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../filter_design/filter_design.hpp"
#include "coeff/martin_coeff.hpp"
#include "iir_base.hpp"
#include "svf_base.hpp"

namespace zldsp::filter {
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
            fs_.store(sampleRate);
            to_update_para_.store(true);
        }

        void setFreq(const FloatType x) {
            freq_.store(x);
            to_update_para_.store(true);
        }

        FloatType getFreq() const { return static_cast<FloatType>(freq_.load()); }

        void setGain(const FloatType x) {
            gain_.store(x);
            to_update_para_.store(true);
        }

        FloatType getGain() const { return static_cast<FloatType>(gain_.load()); }

        void setQ(const FloatType x) {
            q_.store(x);
            to_update_para_.store(true);
        }

        void setFilterType(const FilterType x) {
            filter_type_.store(x);
            to_update_para_.store(true);
        }

        FilterType getFilterType() const { return filter_type_.load(); }

        void setOrder(const size_t x) {
            order_.store(x);
            to_update_para_.store(true);
        }

        void prepareResponseSize(const size_t x) {
            response_.resize(x);
            std::fill(response_.begin(), response_.end(), std::complex(FloatType(1), FloatType(0)));
        }

        bool updateResponse(const std::vector<std::complex<FloatType> > &wis) {
            if (to_update_para_.exchange(false)) {
                updateParas();
                std::fill(response_.begin(), response_.end(), std::complex(FloatType(1), FloatType(0)));
                for (size_t i = 0; i < current_filter_num_; ++i) {
                    IIRBase<FloatType>::updateResponse(coeffs_[i], wis, response_);
                }
                return true;
            }
            return false;
        }

        std::vector<std::complex<FloatType> > &getResponse() { return response_; }

        void setToUpdate() { to_update_para_.store(true); }

    private:
        std::array<std::array<double, 6>, FilterSize> coeffs_{};
        std::atomic<bool> to_update_para_{true};
        std::atomic<size_t> order_{2};
        size_t current_filter_num_{1};
        std::atomic<double> freq_{1000.0}, gain_{0.0}, q_{0.707};
        std::atomic<double> fs_{48000.0};
        std::atomic<FilterType> filter_type_{FilterType::kPeak};
        std::vector<std::complex<FloatType> > response_{};

        void updateParas() {
            current_filter_num_ = updateIIRCoeffs(filter_type_.load(), order_.load(),
                                               freq_.load(), fs_.load(),
                                               gain_.load(), q_.load(), coeffs_);
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
