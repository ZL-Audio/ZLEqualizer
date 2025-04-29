// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../vector/vector.hpp"

namespace zlHistogram {
    template<typename FloatType, size_t Size>
    class Histogram {
    public:
        Histogram() = default;

        /**
         * reset all counts
         */
        void reset(const FloatType x = FloatType(0)) {
            std::fill(hits.begin(), hits.end(), x);
        }

        void setDecayRate(const FloatType x) { decayRate = x; }

        /**
         * add one to bin x
         * @param x bin idx
         */
        void push(size_t x) {
            x = std::min(x, Size - 1);
            hits = hits * decayRate;
            hits[x] = hits[x] + FloatType(1);
        }

        /**
         * get percentile value
         * @param x percentile, 0.05 = 5%, etc
         * @return
         */
        FloatType getPercentile(const FloatType x) {
            cumHits[0] = hits[0];
            for (size_t i = 1; i < Size; ++i) {
                cumHits[i] = cumHits[i - 1] + hits[i];
            }
            const auto targetHits = x * cumHits.back();
            auto it = std::lower_bound(cumHits.begin(), cumHits.end(), targetHits);
            if(it != cumHits.end()) {
                const auto i = static_cast<size_t>(std::distance(cumHits.begin(), it));
                return static_cast<FloatType>(i) + (cumHits[i] - targetHits) / std::max(hits[i], FloatType(1));
            } else {
                return FloatType(1);
            }
        }

        kfr::univector<FloatType, Size> &getHits() { return hits; }

    private:
        kfr::univector<FloatType, Size> hits, cumHits;
        FloatType decayRate{FloatType(0.9997697679981565)}; // np.power(0.1, 1/10000)
    };
}
