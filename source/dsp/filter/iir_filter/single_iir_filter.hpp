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

#include "../filter_design/filter_design.hpp"
#include "../../chore/smoothed_value.hpp"
#include "coeff/martin_coeff.hpp"
#include "iir_base.hpp"

namespace zldsp::filter {
    /**
     * an IIR filter which processes audio on the real-time thread
     * @tparam FloatType the float type of input audio buffer
     * @tparam FilterSize the number of cascading filters
     */
    template<typename FloatType, size_t FilterSize>
    class IIR {
    public:
        IIR() = default;

        void reset() {
            for (size_t i = 0; i < current_filter_num_; ++i) {
                filters_[i].reset();
            }
        }

        void prepare(const double sample_rate, const size_t num_channels) {
            for (auto &f: filters_) {
                f.prepare(num_channels);
            }
            sample_rate_ = sample_rate;
            c_freq_.prepare(sample_rate, 0.1);
            c_gain_.prepare(sample_rate, 0.001);
            c_q_.prepare(sample_rate, 0.001);
            to_update_para_.store(true, std::memory_order::release);
        }

        /**
         * prepare for processing the incoming audio buffer
         * @return
         */
        bool prepareBuffer() {
            bool update = false;
            if (to_update_para_.exchange(false, std::memory_order::acquire)) {
                c_filter_type_ = filter_type_.load(std::memory_order::relaxed);
                c_order_ = order_.load(std::memory_order::relaxed);
                updateCoeffs();
                reset();
                update = true;
            }
            if (to_update_fgq_.exchange(false, std::memory_order::acquire)) {
                c_freq_.setTarget(freq_.load(std::memory_order::relaxed));
                c_gain_.setTarget(gain_.load(std::memory_order::relaxed));
                c_q_.setTarget(q_.load(std::memory_order::relaxed));
                update = true;
            }
            return update;
        }

        /**
         * process the incoming audio buffer
         * @param buffer
         * @param num_samples
         */
        template<bool IsBypassed = false>
        void process(std::span<FloatType *> buffer, const size_t num_samples) {
            if (c_freq_.isSmoothing() || c_gain_.isSmoothing() || c_q_.isSmoothing()) {
                processIIR<IsBypassed, true>(buffer, num_samples);
            } else {
                processIIR<IsBypassed, false>(buffer, num_samples);
            }
        }

        template<bool IsBypassed = false, bool IsSmooth = false>
        void processIIR(std::span<FloatType *> buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                if constexpr (IsSmooth) updateCoeffs();
                for (size_t channel = 0; channel < buffer.size(); ++channel) {
                    auto sample = buffer[channel][i];
                    for (size_t filter_idx = 0; filter_idx < current_filter_num_; ++filter_idx) {
                        sample = filters_[filter_idx].processSample(channel, sample);
                    }
                    if constexpr (!IsBypassed) {
                        buffer[channel][i] = sample;
                    }
                }
            }
        }

        /**
         * set the frequency of the filter
         * @param freq
         */
        template<bool Update = true, bool Async = true, bool Force = false>
        void setFreq(const FloatType freq) {
            if constexpr (Async) {
                freq_.store(static_cast<double>(freq), std::memory_order::relaxed);
                if constexpr (Update) {
                    to_update_fgq_.store(true, std::memory_order::release);
                }
            } else {
                if constexpr (Force) {
                    c_freq_.setCurrentAndTarget(static_cast<double>(freq));
                } else {
                    c_freq_.setTarget(static_cast<double>(freq));
                }
            }
        }

        template<bool Async = true>
        FloatType getFreq() const {
            if constexpr (Async) {
                return static_cast<FloatType>(freq_.load(std::memory_order::relaxed));
            } else {
                return static_cast<FloatType>(c_freq_.getTarget());
            }
        }

        /**
         * set the gain of the filter
         * @param gain
         */
        template<bool Update = true, bool Async = true, bool Force = false>
        void setGain(const FloatType gain) {
            if constexpr (Async) {
                gain_.store(static_cast<double>(gain), std::memory_order::relaxed);
                if constexpr (Update) {
                    to_update_fgq_.store(true, std::memory_order::release);
                }
            } else {
                if constexpr (Force) {
                    c_gain_.setCurrentAndTarget(static_cast<double>(gain));
                } else {
                    c_gain_.setTarget(static_cast<double>(gain));
                }
            }
        }

        template<bool Async = true>
        FloatType getGain() const {
            if constexpr (Async) {
                return static_cast<FloatType>(gain_.load(std::memory_order::relaxed));
            } else {
                return static_cast<FloatType>(c_gain_.getTarget());
            }
        }

        /**
         * set the Q value of the filter
         * @param q
         */
        template<bool Update = true, bool Async = true, bool Force = false>
        void setQ(const FloatType q) {
            if constexpr (Async) {
                q_.store(static_cast<double>(q), std::memory_order::relaxed);
                if constexpr (Update) {
                    to_update_fgq_.store(true, std::memory_order::release);
                }
            } else {
                if constexpr (Force) {
                    c_q_.setCurrentAndTarget(static_cast<double>(q));
                } else {
                    c_q_.setTarget(static_cast<double>(q));
                }
            }
        }

        template<bool Async = true>
        FloatType getQ() const {
            if constexpr (Async) {
                return static_cast<FloatType>(q_.load(std::memory_order::relaxed));
            } else {
                return static_cast<FloatType>(c_q_.getTarget());
            }
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
        template<bool Update = true>
        void setFilterType(const FilterType filter_type) {
            if (filter_type_.exchange(filter_type, std::memory_order::relaxed) != filter_type) {
                if constexpr (Update) {
                    to_update_para_.store(true, std::memory_order::release);
                }
            }
        }

        template<bool Async = true>
        inline FilterType getFilterType() const {
            if constexpr (Async) {
                return filter_type_.load(std::memory_order::relaxed);
            } else {
                return c_filter_type_;
            }
        }

        /**
         * set the order of the filter, the filter will always reset
         * @param order
         */
        template<bool Update = true>
        void setOrder(const size_t order) {
            if (order_.exchange(order, std::memory_order::relaxed) != order) {
                if constexpr (Update) {
                    to_update_para_.store(true, std::memory_order::release);
                }
            }
        }

        template<bool Async = true>
        inline size_t getOrder() const {
            if constexpr (Async) {
                return order_.load(std::memory_order::relaxed);
            } else {
                return c_order_;
            }
        }

        /**
         * update filter coefficients
         * DO NOT call it unless you are sure what you are doing
         */
        void updateCoeffs() {
            const auto next_freq = c_freq_.getNext();
            const auto next_gain = c_gain_.getNext();
            const auto next_q = c_q_.getNext();
            current_filter_num_ = updateIIRCoeffs(c_filter_type_, c_order_,
                                                  next_freq, sample_rate_,
                                                  next_gain, next_q, coeffs_);
            for (size_t i = 0; i < current_filter_num_; ++i) {
                filters_[i].updateFromBiquad(coeffs_[i]);
            }
        }

        /**
         * get the array of 2nd order filters
         * @return
         */
        std::array<IIRBase<FloatType>, FilterSize> &getFilters() {
            return filters_;
        }

    private:
        std::array<IIRBase<FloatType>, FilterSize> filters_{};

        size_t current_filter_num_{1};
        std::atomic<double> freq_{1000.0}, gain_{0.0}, q_{0.707};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kLin> c_gain_{0.0};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kMul> c_q_{0.707};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kFixMul> c_freq_{1000.0};
        std::atomic<size_t> order_{2};
        size_t c_order_{2};
        std::atomic<FilterType> filter_type_{FilterType::kPeak};
        FilterType c_filter_type_{FilterType::kPeak};

        double sample_rate_;

        std::atomic<bool> to_update_para_{true};
        std::atomic<bool> to_update_fgq_{false};

        std::array<std::array<double, 6>, FilterSize> coeffs_{};

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
