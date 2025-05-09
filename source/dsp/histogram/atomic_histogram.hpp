// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include <array>
#include <atomic>

#include "simple_histogram.hpp"

namespace zldsp::histogram {
    template<typename FloatType, size_t Size>
    class AtomicHistogram {
    public:
        AtomicHistogram() = default;

        /**
         * reset all counts
         */
        void reset(const FloatType x = FloatType(0)) {
            syncCount = 0;
            for (auto &hit: hits) {
                hit.store(x);
            }
        }

        void sync(Histogram<FloatType, Size> &simpleHist, const int syncMax = 1000) {
            syncCount += 1;
            if (syncCount > syncMax) {
                const auto simpleHists = simpleHist.getHits();
                for (size_t i = 0; i < hits.size(); ++i) {
                    hits[i].store(simpleHists[i]);
                }
                syncCount = 0;
            }
        }

        /**
         * get percentile value
         * @param x percentile, 0.05 = 5%, etc
         * @return
         */
        FloatType getPercentile(const FloatType x) const {
            std::array<FloatType, Size> cumHits;
            cumHits[0] = hits[0].load();
            for (size_t i = 1; i < Size; ++i) {
                cumHits[i] = cumHits[i - 1] + hits[i].load();
            }
            const auto targetHits = x * cumHits.back();
            auto it = std::lower_bound(cumHits.begin(), cumHits.end(), targetHits);
            if (it != cumHits.end()) {
                const auto i = static_cast<size_t>(std::distance(cumHits.begin(), it));
                return static_cast<FloatType>(i) + (cumHits[i] - targetHits) / std::max(hits[i].load(), FloatType(1));
            } else {
                return FloatType(1);
            }
        }

    private:
        std::array<std::atomic<FloatType>, Size> hits;
        FloatType decayRate{FloatType(0.9997697679981565)}; // np.power(0.1, 1/10000)
        int syncCount{0};
    };
}
