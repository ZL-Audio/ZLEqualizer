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
    template <typename FloatType, size_t kFilterSize>
    class Ideal {
    public:
        Ideal() = default;

        void prepare(const double sample_rate) {
            sample_rate_ = sample_rate;
            freq_max_ = std::floor(sample_rate / 44100.0 * 20000.0);
        }

        void forceUpdate(const FilterParameters& paras) {
            c_filter_type_ = paras.filter_type;
            c_order_ = paras.order;
            c_freq_ = std::min(paras.freq, freq_max_);
            c_gain_ = paras.gain;
            c_q_ = paras.q;
            updateCoeffs();
        }

        void setFreq(const double freq) {
            c_freq_ = std::min(freq, freq_max_);
        }

        [[nodiscard]] double getFreq() const {
            return c_freq_;
        }

        void setGain(const double gain) {
            c_gain_ = gain;
        }

        [[nodiscard]] double getGain() const {
            return c_gain_;
        }

        void setQ(const double q) {
            c_q_ = q;
        }

        [[nodiscard]] double getQ() const {
            return c_q_;
        }

        void setFilterType(const FilterType filter_type) {
            c_filter_type_ = filter_type;
        }

        [[nodiscard]] FilterType getFilterType() const {
            return c_filter_type_;
        }

        void setOrder(const size_t order) {
            c_order_ = order;
        }

        [[nodiscard]] size_t getOrder() const {
            return c_order_;
        }

        void updateCoeffs() {
            current_filter_num_ = updateIIRCoeffs(c_filter_type_, c_order_,
                                                  c_freq_, sample_rate_,
                                                  c_gain_, c_q_,
                                                  coeffs_);
        }

        FloatType getCenterMagnitudeSquare(FloatType w) const {
            auto g0 = static_cast<FloatType>(1.0);
            for (size_t i = 0; i < current_filter_num_; ++i) {
                g0 *= IdealBase::getMagnitudeSquare<FloatType>(coeffs_[i], w);
            }
            return g0;
        }

        void updateMagnitudeSquare(std::span<FloatType> ws, std::span<FloatType> gains) {
            if (current_filter_num_ == 0) {
                std::fill(gains.begin(), gains.end(), static_cast<FloatType>(1.0));
                return;
            }
            IdealBase::updateMagnitudeSquareInplace<FloatType>(coeffs_[0], ws, gains);
            for (size_t i = 1; i < current_filter_num_; ++i) {
                IdealBase::updateMagnitudeSquare<FloatType>(coeffs_[i], ws, gains);
            }
        }

        [[nodiscard]] size_t getFilterNum() const {
            return current_filter_num_;
        }

        std::array<std::array<double, 6>, kFilterSize>& getCoeff() {
            return coeffs_;
        }

        [[nodiscard]] FilterParameters getParas() const {
            return FilterParameters{getFilterType(), getOrder(), getFreq(), getGain(), getQ()};
        }

    protected:
        std::array<std::array<double, 6>, kFilterSize> coeffs_{};
        size_t current_filter_num_{1};
        double freq_max_{20000.0};
        double c_gain_{0.0}, c_q_{0.707}, c_freq_{1000.0};
        size_t c_order_{2};
        FilterType c_filter_type_{FilterType::kPeak};

        double sample_rate_{48000.0};

        static size_t updateIIRCoeffs(const FilterType filter_type, const size_t n,
                                      const double f, const double fs, const double g0, const double q0,
                                      std::array<std::array<double, 6>, kFilterSize>& coeffs) {
            return FilterDesign::updateCoeffs<IdealCoeff>(filter_type, n, f, fs, g0, q0, coeffs);
        }
    };
}
