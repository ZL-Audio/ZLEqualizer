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
     * an empty filter which holds filter parameters
     * @tparam FloatType the float type of input audio buffer
     */
    template<typename FloatType>
    class Empty {
    public:
        explicit Empty() = default;

        void prepare(const double sampleRate) {
            fs_.store(sampleRate, std::memory_order::relaxed);
            to_update_para_.store(true, std::memory_order::release);
        }

        void setFreq(const FloatType x) {
            freq_.store(x, std::memory_order::relaxed);
            to_update_para_.store(true, std::memory_order::release);
        }

        FloatType getFreq() const { return static_cast<FloatType>(freq_.load(std::memory_order::relaxed)); }

        void setGain(const FloatType x) {
            gain_.store(x, std::memory_order::relaxed);
            to_update_para_.store(true, std::memory_order::release);
        }

        FloatType getGain() const { return static_cast<FloatType>(gain_.load(std::memory_order::relaxed)); }

        void setQ(const FloatType x) {
            q_.store(x, std::memory_order::relaxed);
            to_update_para_.store(true, std::memory_order::release);
        }

        FloatType getQ() const { return static_cast<FloatType>(q_.load(std::memory_order::relaxed)); }

        void setFilterType(const FilterType x) {
            filter_type_.store(x, std::memory_order::relaxed);
            to_update_para_.store(true, std::memory_order::release);
        }

        FilterType getFilterType() const { return filter_type_.load(std::memory_order::relaxed); }

        void setOrder(const size_t x) {
            order_.store(x, std::memory_order::relaxed);
            to_update_para_.store(true, std::memory_order::release);
        }

        size_t getOrder() const { return order_.load(std::memory_order::relaxed); }

        std::atomic<bool> &getUpdateParaFlag() { return to_update_para_; }

    private:
        std::atomic<bool> to_update_para_{true};
        std::atomic<size_t> filter_num_{1}, order_{2};
        std::atomic<double> freq_{1000.0}, gain_{0.0}, q_{0.707};
        std::atomic<double> fs_{48000.0};
        std::atomic<FilterType> filter_type_ = FilterType::kPeak;
    };
}
