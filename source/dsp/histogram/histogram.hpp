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
    template<typename FloatType>
    class Histogram {
    public:
        Histogram() = default;

        void reset();

        void push(size_t x);

        void setSize(size_t x);

        FloatType getPercentile(FloatType x) const;

    private:
        std::vector<FloatType> hits;
        juce::ReadWriteLock histLock;
    };
}


#endif //ZLEqualizer_HISTOGRAM_HPP
