// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "ideal_base.hpp"
#include "coeff/ideal_coeff.hpp"
#include "../filter_design/filter_design.hpp"
#include "../../chore/decibels.hpp"

namespace zldsp::filter {
    /**
     * an ideal prototype filter which holds coeffs for calculating responses
     * @tparam FloatType the float type of input audio buffer
     * @tparam kFilterSize the number of cascading filters
     */
    template<typename FloatType, size_t kFilterSize>
    class Ideal {
    public:
        explicit Ideal() = default;

        void prepare(const double sample_rate) {
            sample_rate_ = sample_rate;
        }

        void forceUpdate(const FilterParameters &paras) {
            c_filter_type_ = paras.filter_type;
            c_order_ = paras.order;
            c_freq_ = paras.freq;
            c_gain_ = paras.gain;
            c_q_ = paras.q;
            updateCoeffs();
        }

        void setFreq(const double freq) {
            c_freq_ = freq;
        }

        double getFreq() const {
            return c_freq_;
        }

        void setGain(const double gain) {
            c_gain_ = gain;
        }

        double getGain() const {
            return c_gain_;
        }

        void setQ(const double q) {
            c_q_ = q;
        }

        double getQ() const {
            return c_q_;
        }

        void setFilterType(const FilterType filter_type) {
            c_filter_type_ = filter_type;
        }

        FilterType getFilterType() const {
            return c_filter_type_;
        }

        void setOrder(const size_t order) {
            c_order_ = order;
        }

        size_t getOrder() const {
            return c_order_;
        }

        void updateCoeffs() {
            current_filter_num_ = updateIIRCoeffs(c_filter_type_, c_order_,
                                                  c_freq_, sample_rate_,
                                                  c_gain_, c_q_,
                                                  coeffs_);
        }

        FloatType getDB(FloatType w) const {
            double g0 = 1.0;
            for (size_t i = 0; i < current_filter_num_; ++i) {
                g0 *= IdealBase<FloatType>::getMagnitude(coeffs_[i], w);
            }
            return static_cast<FloatType>(chore::gainToDecibels(g0));
        }

        [[nodiscard]] size_t getFilterNum() const {
            return current_filter_num_;
        }

        std::array<std::array<double, 6>, kFilterSize> &getCoeff() {
            return coeffs_;
        }

    protected:
        std::array<std::array<double, 6>, kFilterSize> coeffs_{};
        size_t current_filter_num_{1};
        double c_gain_{0.0}, c_q_{0.707}, c_freq_{1000.0};
        size_t c_order_{2};
        FilterType c_filter_type_{FilterType::kPeak};

        double sample_rate_{48000.0};

        static size_t updateIIRCoeffs(const FilterType filter_type, const size_t n,
                                      const double f, const double fs, const double g0, const double q0,
                                      std::array<std::array<double, 6>, kFilterSize> &coeffs) {
            return FilterDesign::updateCoeffs<IdealCoeff>(filter_type, n, f, fs, g0, q0, coeffs);
        }
    };
}
