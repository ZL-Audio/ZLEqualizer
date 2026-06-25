// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <algorithm>

#include "../vector/vector.hpp"

namespace zldsp::histogram {
    template <typename FloatType>
    class Histogram {
    public:
        Histogram(FloatType min_val, FloatType max_val, size_t num_bin)
            : min_val_(min_val), max_val_(max_val) {
            bins_.resize(num_bin);
            bin_width_ = (max_val_ - min_val_) / static_cast<FloatType>(num_bin);
            inv_bin_width_ = static_cast<FloatType>(1.0) / bin_width_;
            bin_max_index_ = static_cast<FloatType>(num_bin - 1);
            reset();
        }

        void setDecay(const double decay) {
            decay_ = decay;
        }

        void reset() {
            std::ranges::fill(bins_, 0.);
            total_count_ = 0.;
        }

        void push(const FloatType value) {
            if (std::isnan(value)) {
                return;
            }

            vector::multiply(bins_.data(), decay_, bins_.size());
            const FloatType raw_index = (value - min_val_) * inv_bin_width_;
            const FloatType clamped_index = std::clamp(raw_index, static_cast<FloatType>(0), bin_max_index_);
            bins_[static_cast<size_t>(clamped_index)] += 1.0;

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

        /**
         * get the approximate values at multiple given percentiles
         * @param ps sorted percentiles, 0.0 < ps < 1.0
         * @param target_counts temp buffer for storing target counts
         * @param results
         * @return
         */
        void getPercentiles(std::span<double> ps,
                            std::span<double> target_counts,
                            std::span<FloatType> results) const {
            // calculate target counts
            for (size_t i = 0; i < ps.size(); ++i) {
                target_counts[i] = total_count_ * ps[i];
            }
            double current_count{0.};
            size_t p_idx{0};
            for (size_t i = 0; i < bins_.size(); ++i) {
                current_count += bins_[i];
                while (current_count >= target_counts[p_idx]) {
                    const auto fraction = bins_[i] > 1e-3
                                              ? (target_counts[p_idx] - (current_count - bins_[i])) / bins_[i]
                                              : 0.0;
                    results[p_idx] = min_val_ + static_cast<FloatType>(static_cast<double>(i) + fraction) * bin_width_;
                    p_idx += 1;
                    if (p_idx == ps.size()) {
                        return;
                    }
                }
            }
            for (; p_idx < ps.size(); ++p_idx) {
                results[p_idx] = max_val_;
            }
        }

    private:
        FloatType min_val_, max_val_;
        vector::aligned_vector<double> bins_;
        FloatType bin_width_, inv_bin_width_;
        FloatType bin_max_index_;
        double decay_{};
        double total_count_{};
    };
}
