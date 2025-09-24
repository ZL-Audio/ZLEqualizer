// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../vector/kfr_import.hpp"

namespace zldsp::histogram {
    template<typename FloatType>
    class Histogram {
    public:
        Histogram(FloatType min_val, FloatType max_val, size_t num_bin)
            : min_val_(min_val), max_val_(max_val) {
            bins_.resize(num_bin);
            bin_width_ = (max_val_ - min_val_) / static_cast<FloatType>(num_bin);
            reset();
        }

        void setDecay(const double decay) {
            decay_ = decay;
        }

        void reset() {
            std::fill(bins_.begin(), bins_.end(), 0.);
            total_count_ = 0.;
        }

        void push(const FloatType value) {
            if (value < min_val_ || value >= max_val_) {
                return;
            }
            const auto bin_index = static_cast<size_t>((value - min_val_) / bin_width_);
            bins_ = bins_ * decay_;
            bins_[bin_index] += 1.0;

            total_count_ = total_count_ * decay_ + 1.0;
        }

        /**
         * get the approximate value at a given percentile
         * @param p 0.0 < p < 1.0
         * @return
         */
        FloatType getPercentile(const double p) const {
            const auto target_count = total_count_ * p;
            double current_count{0.};
            for (size_t i = 0; i < bins_.size(); ++i) {
                current_count += bins_[i];
                if (current_count >= target_count) {
                    const auto fraction = (target_count - (current_count - bins_[i])) / bins_[i];
                    return min_val_ + static_cast<FloatType>(static_cast<double>(i) + fraction) * bin_width_;
                }
            }
            return max_val_;
        }

    private:
        FloatType min_val_, max_val_;
        kfr::univector<double> bins_;
        FloatType bin_width_;
        double decay_{};
        double total_count_{};
    };
}
