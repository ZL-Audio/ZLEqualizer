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

#include "../../filter_design/filter_design.hpp"

namespace zldsp::filter {
    /**
     * an empty IIR filter which holds filter parameters
     */
    class IIREmpty {
    public:
        IIREmpty() = default;

        bool getToUpdatePara() {
            return to_update_para_.exchange(false, std::memory_order::acquire);
        }

        bool getToUpdateFGQ() {
            return to_update_fgq_.exchange(false, std::memory_order::acquire);
        }

        template<bool Update = true>
        void setFreq(const double freq) {
            freq_.store(freq, std::memory_order::relaxed);
            if constexpr (Update) {
                to_update_fgq_.store(true, std::memory_order::release);
            }
        }

        double getFreq() const {
            return freq_.load(std::memory_order::relaxed);
        }

        /**
         * set the gain of the filter
         * @param gain
         */
        template<bool Update = true>
        void setGain(const double gain) {
            gain_.store(gain, std::memory_order::relaxed);
            if constexpr (Update) {
                to_update_fgq_.store(true, std::memory_order::release);
            }
        }

        double getGain() const {
            return gain_.load(std::memory_order::relaxed);
        }

        /**
         * set the Q value of the filter
         * @param q
         */
        template<bool Update = true>
        void setQ(const double q) {
            q_.store(q, std::memory_order::relaxed);
            if constexpr (Update) {
                to_update_fgq_.store(true, std::memory_order::release);
            }
        }

        double getQ() const {
            return q_.load(std::memory_order::relaxed);
        }

        template<bool Update = true>
        void setFilterType(const FilterType filter_type) {
            filter_type_.store(filter_type, std::memory_order::relaxed);
            if constexpr (Update) {
                to_update_para_.store(true, std::memory_order::release);
            }
        }

        FilterType getFilterType() const {
            return filter_type_.load(std::memory_order::relaxed);
        }

        template<bool Update = true>
        void setOrder(const size_t order) {
            order_.store(order, std::memory_order::relaxed);
            if constexpr (Update) {
                to_update_para_.store(true, std::memory_order::release);
            }
        }

        size_t getOrder() const {
            return order_.load(std::memory_order::relaxed);
        }

    private:
        std::atomic<double> freq_{1000.0}, gain_{0.0}, q_{0.707};
        std::atomic<size_t> order_{2};
        std::atomic<FilterType> filter_type_{FilterType::kPeak};

        std::atomic<bool> to_update_para_{true};
        std::atomic<bool> to_update_fgq_{false};
    };
}
