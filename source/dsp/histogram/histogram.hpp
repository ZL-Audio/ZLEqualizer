// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_HISTOGRAM_HPP
#define ZLEqualizer_HISTOGRAM_HPP

#include <juce_dsp/juce_dsp.h>

namespace zlHistogram {
    /**
     * a lock free, thread safe histogram
     * @tparam FloatType count precision
     * @tparam Size size of the histogram
     */
    template<typename FloatType, size_t Size>
    class Histogram {
    public:
        Histogram() = default;

        /**
         * reset all counts
         */
        void reset(const FloatType x = FloatType(0)) {
            for (auto &hit: hits) {
                hit.store(FloatType(x));
            }
        }

        void setDecayRate(const FloatType x) { decayRate = x; }

        /**
         * add one to bin x
         * @param x bin idx
         */
        void push(size_t x) {
            x = std::min(x, Size - 1);
            for (auto &hit: hits) {
                hit.store(hit.load() * decayRate);
            }
            hits[x].store(hits[x].load() + FloatType(1));
        }

        /**
         * get percentile value
         * @param x percentile, 0.05 = 5%, etc
         * @return
         */
        FloatType getPercentile(const FloatType x) const {
            FloatType totoalHits = 0;
            for (auto &hit: hits) {
                totoalHits += hit.load();
            }
            const FloatType targetHits = x * totoalHits;
            FloatType currentHits = 0;
            for (size_t i = 0; i < hits.size(); ++i) {
                const auto hit = hits[i].load();
                currentHits += hit;
                if (currentHits >= targetHits) {
                    return static_cast<FloatType>(i) + (currentHits - targetHits) / std::max(hit, FloatType(1));
                }
            }
            return FloatType(1);
        }

    private:
        std::array<std::atomic<FloatType>, Size> hits;
        FloatType decayRate{FloatType(0.9997697679981565)}; // np.power(0.1, 1/10000)
    };
}


#endif //ZLEqualizer_HISTOGRAM_HPP
