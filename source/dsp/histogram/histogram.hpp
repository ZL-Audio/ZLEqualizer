// Copyright (C) 2023 - zsliu98
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
    template<typename FloatType, size_t Size>
    class Histogram {
    public:
        Histogram() = default;

        void reset()  {
            for (auto & hit : hits) {
                hit.store(0);
            }
        }

        void push(const size_t x) {
            hits[x].fetch_add(1);
        }

        FloatType getPercentile(const FloatType x) const {
            FloatType totoalHits = 0;
            for (auto & hit : hits) {
                totoalHits += static_cast<FloatType>(hit.load());
            }
            const FloatType targetHits = x * totoalHits;
            FloatType currentHits = 0;
            for (size_t i = 0; i < hits.size(); ++i) {
                const auto hit = static_cast<FloatType>(hits[i].load());
                currentHits += hit;
                if (currentHits >= targetHits) {
                    return static_cast<FloatType>(i) + (currentHits - targetHits) / std::max(hit, FloatType(1));
                }
            }
            return FloatType(1);
        }

    private:
        std::array<std::atomic<size_t>, Size> hits;
    };
}


#endif //ZLEqualizer_HISTOGRAM_HPP
