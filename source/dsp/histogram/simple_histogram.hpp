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

namespace zldsp::histogram {
    template<typename FloatType, size_t Size>
    class Histogram {
    public:
        Histogram() = default;

        /**
         * reset all counts
         */
        void reset(const FloatType x = FloatType(0)) {
            std::fill(hits_.begin(), hits_.end(), x);
        }

        void setDecayRate(const FloatType x) { decay_rate_ = x; }

        /**
         * add one to bin x
         * @param x bin idx
         */
        void push(size_t x) {
            x = std::min(x, Size - 1);
            hits_ = hits_ * decay_rate_;
            hits_[x] = hits_[x] + FloatType(1);
        }

        /**
         * get percentile value
         * @param x percentile, 0.05 = 5%, etc
         * @return
         */
        FloatType getPercentile(const FloatType x) {
            cum_hits_[0] = hits_[0];
            for (size_t i = 1; i < Size; ++i) {
                cum_hits_[i] = cum_hits_[i - 1] + hits_[i];
            }
            const auto target_hits = x * cum_hits_.back();
            auto it = std::lower_bound(cum_hits_.begin(), cum_hits_.end(), target_hits);
            if(it != cum_hits_.end()) {
                const auto i = static_cast<size_t>(std::distance(cum_hits_.begin(), it));
                return static_cast<FloatType>(i) + (cum_hits_[i] - target_hits) / std::max(hits_[i], FloatType(1));
            } else {
                return FloatType(1);
            }
        }

        kfr::univector<FloatType, Size> &getHits() { return hits_; }

    private:
        kfr::univector<FloatType, Size> hits_, cum_hits_;
        FloatType decay_rate_{FloatType(0.9997697679981565)}; // np.power(0.1, 1/10000)
    };
}
