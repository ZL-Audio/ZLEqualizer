// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <span>

#include "../../filter_design/filter_design.hpp"
#include "../../../chore/smoothed_value.hpp"
#include "../coeff/martin_coeff.hpp"
#include "iir_empty.hpp"

namespace zldsp::filter {
    /**
     * an IIR filter which processes audio on the real-time thread
     * @tparam FloatType the float type of input audio buffer
     * @tparam FilterSize the number of cascading filters
     */
    template<typename FloatType, size_t FilterSize>
    class IIR {
    public:
        explicit IIR(IIREmpty &empty) : empty_(empty) {
        }

        virtual ~IIR() = default;

        virtual void reset() = 0;

        virtual void prepare(double sample_rate, size_t num_channels, size_t max_num_samples) = 0;

        void prepareSampleRate(const double sample_rate) {
            sample_rate_ = sample_rate;
            c_freq_.prepare(sample_rate, 0.1);
            c_gain_.prepare(sample_rate, 0.001);
            c_q_.prepare(sample_rate, 0.001);
            updateCoeffs();
        }

        /**
         * prepare for processing the incoming audio buffer
         * @return
         */
        bool prepareBuffer() {
            bool update = false;
            if (empty_.getToUpdatePara()) {
                c_filter_type_ = empty_.getFilterType();
                c_order_ = empty_.getOrder();
                updateCoeffs();
                reset();
                update = true;
            }
            if (empty_.getToUpdateFGQ()) {
                c_freq_.setTarget(empty_.getFreq());
                c_gain_.setTarget(empty_.getGain());
                c_q_.setTarget(empty_.getQ());
                update = true;
            }
            return update;
        }

        /**
         * set the frequency of the filter
         * @param freq
         */
        template<bool Force = false>
        void setFreq(const FloatType freq) {
            if constexpr (Force) {
                c_freq_.setCurrentAndTarget(static_cast<double>(freq));
            } else {
                c_freq_.setTarget(static_cast<double>(freq));
            }
        }

        FloatType getFreq() const {
            return static_cast<FloatType>(c_freq_.getTarget());
        }

        /**
         * set the gain of the filter
         * @param gain
         */
        template<bool Force = false>
        void setGain(const FloatType gain) {
            if constexpr (Force) {
                c_gain_.setCurrentAndTarget(static_cast<double>(gain));
            } else {
                c_gain_.setTarget(static_cast<double>(gain));
            }
        }

        FloatType getGain() const {
            return static_cast<FloatType>(c_gain_.getTarget());
        }

        /**
         * set the Q value of the filter
         * @param q
         */
        template<bool Force = false>
        void setQ(const FloatType q) {
            if constexpr (Force) {
                c_q_.setCurrentAndTarget(static_cast<double>(q));
            } else {
                c_q_.setTarget(static_cast<double>(q));
            }
        }

        FloatType getQ() const {
            return static_cast<FloatType>(c_q_.getTarget());
        }

        void skipSmooth() {
            c_freq_.setCurrentAndTarget(c_freq_.getTarget());
            c_gain_.setCurrentAndTarget(c_gain_.getTarget());
            c_q_.setCurrentAndTarget(c_q_.getTarget());
            updateCoeffs();
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

    protected:
        IIREmpty &empty_;

        size_t current_filter_num_{1};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kLin> c_gain_{0.0};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kMul> c_q_{0.707};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kFixMul> c_freq_{1000.0};
        size_t c_order_{2};
        FilterType c_filter_type_{FilterType::kPeak};

        double sample_rate_{48000.0};

        std::array<std::array<double, 6>, FilterSize> coeffs_{};
    };
}
