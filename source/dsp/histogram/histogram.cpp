// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "histogram.hpp"

namespace zlHistogram {
    template<typename FloatType>
    void Histogram<FloatType>::reset() {
        juce::ScopedWriteLock lock(histLock);
        std::fill(hits.begin(), hits.end(), FloatType(0));
    }

    template<typename FloatType>
    void Histogram<FloatType>::push(const size_t x) {
        juce::ScopedWriteLock lock(histLock);
        hits[x] += FloatType(1);
    }

    template<typename FloatType>
    void Histogram<FloatType>::setSize(size_t x) {
        juce::ScopedWriteLock lock(histLock);
        std::fill(hits.begin(), hits.end(), FloatType(0));
        hits.resize(x);
    }

    template<typename FloatType>
    FloatType Histogram<FloatType>::getPercentile(const FloatType x) const {
        juce::ScopedReadLock lock(histLock);
        const FloatType totoalHits = std::reduce(hits.begin(), hits.end());
        const FloatType targetHits = x * totoalHits;
        FloatType currentHits = 0;
        for (size_t i = 0; i < hits.size(); ++i) {
            currentHits += hits[i];
            if (currentHits >= targetHits) {
                return static_cast<FloatType>(i) + (currentHits - targetHits) / std::max(hits[i], FloatType(1));
            }
        }
        return FloatType(1);
    }

    template
    class Histogram<float>;

    template
    class Histogram<double>;
}
