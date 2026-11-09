// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../filter_design/filter_design.hpp"
#if ZL_ENABLE_FILTER_SCALED_PASS
#include "../../scaled_pass/design.hpp"
#endif
#include "../../../chore/smoothed_value.hpp"

namespace zldsp::filter {
    /**
     * an IIR filter which processes audio on the real-time thread
     * @tparam kFilterSize the number of cascading filters
     */
    template <size_t kFilterSize, class Coeff>
    class IIR {
    public:
        IIR() = default;

        virtual ~IIR() = default;

        virtual void reset() = 0;

        virtual void prepare(double sample_rate, size_t num_channels, size_t max_num_samples) = 0;

        /**
         * prepare the filter and parameter smoothing
         * @param sample_rate a finite, positive sample rate in Hz
         */
        void prepareSampleRate(const double sample_rate) {
            sample_rate_ = sample_rate;
            c_freq_.prepare(sample_rate, 0.01);
            c_gain_.prepare(sample_rate, 0.001);
            c_q_.prepare(sample_rate, 0.001);
#if ZL_ENABLE_FILTER_SCALED_PASS
            c_scale_.prepare(sample_rate, 0.01);
            c_scale_.setCurrentAndTarget(c_scale_.getTarget());
#endif
            c_freq_.setCurrentAndTarget(c_freq_.getTarget());
            updateCoeffs();
            cacheDynPara();
        }

        /**
         * update filter parameters
         * @return
         */
        void forceUpdate(const FilterParameters& paras) {
#if ZL_ENABLE_FILTER_SCALED_PASS
            const auto reset_state = (isScaledPass(c_filter_type_) || isScaledPass(paras.filter_type))
                && (c_filter_type_ != paras.filter_type || c_order_ != paras.order);
#endif
            c_filter_type_ = paras.filter_type;
            c_order_ = paras.order;
            c_freq_.setCurrentAndTarget(paras.freq);
            c_gain_.setCurrentAndTarget(paras.gain);
            c_q_.setCurrentAndTarget(paras.q);
#if ZL_ENABLE_FILTER_SCALED_PASS
            c_scale_.setCurrentAndTarget(c_scale_.getTarget());
#endif
            updateCoeffs();
#if ZL_ENABLE_FILTER_SCALED_PASS
            if (reset_state) { reset(); }
#endif
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
            c_freq_.setTarget(paras.freq);
            c_gain_.setTarget(paras.gain);
            c_q_.setTarget(paras.q);
        }

        /**
         * set the frequency of the filter
         * @param freq
         */
        template <bool force = false>
        void setFreq(const double freq) {
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

#if ZL_ENABLE_FILTER_SCALED_PASS
        /**
         * set the magnitude scale of a scaled low/high-pass filter
         * @param scale a finite magnitude scale in [0, 1]
         */
        template <bool force = false>
        void setScale(const double scale) {
            if constexpr (force) {
                c_scale_.setCurrentAndTarget(scale);
            } else {
                c_scale_.setTarget(scale);
                if (!c_scale_.isSmoothing()) {
                    c_scale_.setCurrentAndTarget(scale);
                    if (isScaledPass(c_filter_type_)) {
                        updateCoeffs();
                    }
                }
            }
        }

        [[nodiscard]] double getScale() const { return c_scale_.getTarget(); }
#endif
        [[nodiscard]] bool isSmoothing() const {
#if ZL_ENABLE_FILTER_SCALED_PASS
            if (isScaledPass(c_filter_type_) && c_scale_.isSmoothing()) { return true; }
#endif
            return c_freq_.isSmoothing() || c_gain_.isSmoothing() || c_q_.isSmoothing();
        }

        void advanceSmoothing() {
            c_freq_.getNext();
            c_gain_.getNext();
            c_q_.getNext();
#if ZL_ENABLE_FILTER_SCALED_PASS
            if (isScaledPass(c_filter_type_)) {
                c_scale_.getNext();
                if (!c_scale_.isSmoothing()) { c_scale_.setCurrentAndTarget(c_scale_.getTarget()); }
            }
#endif
        }

#if ZL_ENABLE_FILTER_SCALED_PASS
        const scaled_pass::Coefficients& getScaledCoefficients() const {
            return scaled_design_.getCoefficients();
        }
#endif

        [[nodiscard]] bool isFreqQSmoothing() const {
            return this->c_freq_.isSmoothing() || this->c_q_.isSmoothing();
        }

        void skipSmooth() {
            c_freq_.setCurrentAndTarget(c_freq_.getTarget());
            c_gain_.setCurrentAndTarget(c_gain_.getTarget());
            c_q_.setCurrentAndTarget(c_q_.getTarget());
#if ZL_ENABLE_FILTER_SCALED_PASS
            c_scale_.setCurrentAndTarget(c_scale_.getTarget());
#endif
        }

        /**
         * set the type of the filter, the filter will always reset
         * @param filter_type
         */
        void setFilterType(const FilterType filter_type) {
#if ZL_ENABLE_FILTER_SCALED_PASS
            if (c_filter_type_ != filter_type && (isScaledPass(c_filter_type_) || isScaledPass(filter_type))) {
                reset();
            }
#endif
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
#if ZL_ENABLE_FILTER_SCALED_PASS
            if (c_order_ != order && isScaledPass(c_filter_type_)) { reset(); }
#endif
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
         * refresh dynamic coefficients' sample-rate-dependent parameters
         */
        virtual void cacheDynPara() = 0;

        [[nodiscard]] size_t getFilterNum() const {
            return current_filter_num_;
        }

        std::array<std::array<double, 5>, kFilterSize>& getCoeff() {
            return coeffs_;
        }

    protected:
        size_t getProcessingOrder() const {
#if ZL_ENABLE_FILTER_SCALED_PASS
            if (isScaledPass(c_filter_type_)) { return 0; }
#endif
            return c_filter_type_ == kFlatTilt || c_filter_type_ == kFlatGain ? 0 : c_order_;
        }

        void designCoeffs() {
#if ZL_ENABLE_FILTER_SCALED_PASS
            if (isScaledPass(c_filter_type_)) {
                scaled_design_.update(c_filter_type_, c_order_, c_freq_.getCurrent(), sample_rate_,
                                      c_q_.getCurrent(), c_scale_.getCurrent());
                current_filter_num_ = scaled_design_.getCoefficients().size();
                return;
            }
#endif
            current_filter_num_ = FilterDesign::updateCoeffs<Coeff>(
                c_filter_type_, c_order_, c_freq_.getCurrent(), sample_rate_,
                c_gain_.getCurrent(), c_q_.getCurrent(), coeffs_);
        }

#if ZL_ENABLE_FILTER_SCALED_PASS
        scaled_pass::Designer<Coeff> scaled_design_{};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kLin> c_scale_{1.0};
#endif
        std::array<std::array<double, 5>, kFilterSize> coeffs_{};
        size_t current_filter_num_{1};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kLin> c_gain_{0.0};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kMul> c_q_{0.707};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kFixMul> c_freq_{1000.0};
        size_t c_order_{2};
        FilterType c_filter_type_{FilterType::kPeak};
        std::array<double, kFilterSize * 3 + 1> cache_{};

        double sample_rate_{48000.0};
    };
}
