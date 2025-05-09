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
#include <atomic>

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
            fs.store(sampleRate);
            toUpdatePara.store(true);
        }

        void setFreq(const FloatType x) {
            freq.store(x);
            toUpdatePara.store(true);
        }

        FloatType getFreq() const { return static_cast<FloatType>(freq.load()); }

        void setGain(const FloatType x) {
            gain.store(x);
            toUpdatePara.store(true);
        }

        FloatType getGain() const { return static_cast<FloatType>(gain.load()); }

        void setQ(const FloatType x) {
            q.store(x);
            toUpdatePara.store(true);
        }

        FloatType getQ() const { return static_cast<FloatType>(q.load()); }

        void setFilterType(const FilterType x) {
            filterType.store(x);
            toUpdatePara.store(true);
        }

        FilterType getFilterType() const { return filterType.load(); }

        void setOrder(const size_t x) {
            order.store(x);
            toUpdatePara.store(true);
        }

        size_t getOrder() const { return order.load(); }

        bool getParaOutdated() const { return toUpdatePara.load(); }

        bool exchangeParaOutdated(const bool x) { return toUpdatePara.exchange(x); }

    private:
        std::atomic<bool> toUpdatePara{true};
        std::atomic<size_t> filterNum{1}, order{2};
        std::atomic<double> freq{1000.0}, gain{0.0}, q{0.707};
        std::atomic<double> fs{48000.0};
        std::atomic<FilterType> filterType = FilterType::kPeak;
    };
}
