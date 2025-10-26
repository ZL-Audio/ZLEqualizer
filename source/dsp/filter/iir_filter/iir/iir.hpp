// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../filter_design/filter_design.hpp"
#include "../coeff/martin_coeff.hpp"
#include "../../../chore/smoothed_value.hpp"

namespace zldsp::filter {
    /**
     * an IIR filter which processes audio on the real-time thread
     * @tparam kFilterSize the number of cascading filters
     */
    template <size_t kFilterSize>
    class IIR {
    public:
        IIR() = default;

        virtual ~IIR() = default;

        virtual void reset() = 0;

        virtual void prepare(double sample_rate, size_t num_channels, size_t max_num_samples) = 0;

        void prepareSampleRate(const double sample_rate) {
            sample_rate_ = sample_rate;
            c_freq_.prepare(sample_rate, 0.1);
            c_gain_.prepare(sample_rate, 0.001);
            c_q_.prepare(sample_rate, 0.001);
            freq_max_ = std::floor(sample_rate / 44100.0 * 20000.0);
            c_freq_.setCurrentAndTarget(std::min(c_freq_.getTarget(), freq_max_));
            updateCoeffs();
        }

        /**
         * update filter parameters
         * @return
         */
        void forceUpdate(const FilterParameters& paras) {
            c_filter_type_ = paras.filter_type;
            c_order_ = paras.order;
            c_freq_.setCurrentAndTarget(std::min(paras.freq, freq_max_));
            c_gain_.setCurrentAndTarget(paras.gain);
            c_q_.setCurrentAndTarget(paras.q);
            updateCoeffs();
        }

        /**
         * update filter parameters
         * @return
         */
        void updateParas(const FilterParameters& paras) {
            if (paras.filter_type != c_filter_type_ || paras.order != c_order_) {
                c_filter_type_ = paras.filter_type;
                c_order_ = paras.order;
                updateCoeffs();
                reset();
            }
            c_freq_.setTarget(std::min(paras.freq, freq_max_));
            c_gain_.setTarget(paras.gain);
            c_q_.setTarget(paras.q);
        }

        /**
         * set the frequency of the filter
         * @param freq
         */
        template <bool force = false>
        void setFreq(double freq) {
            freq = std::min(freq, freq_max_);
            if constexpr (force) {
                c_freq_.setCurrentAndTarget(freq);
            } else {
                c_freq_.setTarget(freq);
            }
        }

        [[nodiscard]] double getFreq() const {
            return c_freq_.getTarget();
        }

        /**
         * set the gain of the filter
         * @param gain
         */
        template <bool force = false>
        void setGain(const double gain) {
            if constexpr (force) {
                c_gain_.setCurrentAndTarget(gain);
            } else {
                c_gain_.setTarget(gain);
            }
        }

        [[nodiscard]] double getGain() const {
            return c_gain_.getTarget();
        }

        /**
         * set the Q value of the filter
         * @param q
         */
        template <bool force = false>
        void setQ(const double q) {
            if constexpr (force) {
                c_q_.setCurrentAndTarget(q);
            } else {
                c_q_.setTarget(q);
            }
        }

        [[nodiscard]] double getQ() const {
            return c_q_.getTarget();
        }

        bool isFreqQSmoothing() const {
            return this->c_freq_.isSmoothing() || this->c_q_.isSmoothing();
        }

        void skipSmooth() {
            c_freq_.setCurrentAndTarget(c_freq_.getTarget());
            c_gain_.setCurrentAndTarget(c_gain_.getTarget());
            c_q_.setCurrentAndTarget(c_q_.getTarget());
        }

        /**
         * set the type of the filter, the filter will always reset
         * @param filter_type
         */
        void setFilterType(const FilterType filter_type) {
            c_filter_type_ = filter_type;
        }

        [[nodiscard]] FilterType getFilterType() const {
            return c_filter_type_;
        }

        /**
         * set the order of the filter, the filter will always reset
         * @param order
         */
        void setOrder(const size_t order) {
            c_order_ = order;
        }

        [[nodiscard]] size_t getOrder() const {
            return c_order_;
        }

        /**
         * update filter coefficients
         */
        virtual void updateCoeffs() = 0;

        /**
         * update filter gain
         */
        virtual void updateGain() = 0;

        size_t getFilterNum() const {
            return current_filter_num_;
        }

        std::array<std::array<double, 6>, kFilterSize>& getCoeff() {
            return coeffs_;
        }

    protected:
        std::array<std::array<double, 6>, kFilterSize> coeffs_{};
        size_t current_filter_num_{1};
        double freq_max_{20000.0};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kLin> c_gain_{0.0};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kMul> c_q_{0.707};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kFixMul> c_freq_{1000.0};
        size_t c_order_{2};
        FilterType c_filter_type_{FilterType::kPeak};

        double sample_rate_{48000.0};

        static size_t updateIIRCoeffs(const FilterType filter_type, const size_t n,
                                      const double f, const double fs, const double g0, const double q0,
                                      std::array<std::array<double, 6>, kFilterSize>& coeffs) {
            return FilterDesign::updateCoeffs<MartinCoeff>(filter_type, n, f, fs, g0, q0, coeffs);
        }
    };
}
