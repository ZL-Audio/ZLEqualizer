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

#include "../helpers.hpp"

namespace zldsp::filter {
    /**
     * an empty IIR filter which holds atomic filter parameters
     */
    class Empty {
    public:
        Empty() = default;

        void setFreq(const double x) {
            freq_.store(x, std::memory_order::relaxed);
        }

        [[nodiscard]] double getFreq() const {
            return freq_.load(std::memory_order::relaxed);
        }

        void setGain(const double x) {
            gain_.store(x, std::memory_order::relaxed);
        }

        [[nodiscard]] double getGain() const {
            return gain_.load(std::memory_order::relaxed);
        }

        void setQ(const double x) {
            q_.store(x, std::memory_order::relaxed);
        }

        [[nodiscard]] double getQ() const {
            return q_.load(std::memory_order::relaxed);
        }

        void setFilterType(const FilterType x) {
            filter_type_.store(x, std::memory_order::relaxed);
        }

        [[nodiscard]] FilterType getFilterType() const {
            return filter_type_.load(std::memory_order::relaxed);
        }

        void setOrder(const size_t x) {
            order_.store(x, std::memory_order::relaxed);
        }

        [[nodiscard]] size_t getOrder() const {
            return order_.load(std::memory_order::relaxed);
        }

        [[nodiscard]] FilterParameters getParas() const {
            return FilterParameters{getFilterType(), getOrder(), getFreq(), getGain(), getQ()};
        }

    private:
        std::atomic<double> freq_{1000.0}, gain_{0.0}, q_{0.707};
        std::atomic<size_t> order_{2};
        std::atomic<FilterType> filter_type_{FilterType::kPeak};
    };
}
