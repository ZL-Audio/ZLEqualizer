// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include <vector>
#include <cassert>

namespace zldsp::analyzer {
    class SpectrumSmoother {
    public:
        explicit SpectrumSmoother() = default;

        void prepare(const size_t fft_size) {
            assert(fft_size != 0);
            low_idx_.resize(fft_size / 2 + 1);
            high_idx_.resize(fft_size / 2 + 1);
            count_req_.resize(fft_size / 2 + 1);
            temp_cum_sum_.resize(fft_size / 2 + 2);
        }

        void setSmooth(const double smooth_oct) {
            assert(low_idx_.size() != 0);
            const double factor = std::pow(2.0, smooth_oct * 0.5);
            const double factor_rep = 1.0 / factor;
            const size_t max_idx = low_idx_.size();

            for (size_t i = 0; i < low_idx_.size(); ++i) {
                const double lower = static_cast<double>(i) * factor_rep;
                const double upper = static_cast<double>(i) * factor;

                low_idx_[i] = static_cast<size_t>(std::round(lower));
                high_idx_[i] = std::min(max_idx, static_cast<size_t>(std::round(upper) + 1.0));

                const size_t bin_count = high_idx_[i] - low_idx_[i];
                count_req_[i] = 1.0f / static_cast<float>(std::max<size_t>(1, bin_count));
            }
        }

        void setSmoothERB(const double sample_rate, const double smooth_erb = 1.0) {
            assert(low_idx_.size() != 0);
            assert(sample_rate > 0.0);

            const auto factor = smooth_erb * 0.5;
            const auto num_bins = low_idx_.size();
            const auto fft_size = (num_bins - 1) * 2;
            const auto delta_f = sample_rate / static_cast<double>(fft_size);
            const auto const_term = 24.7 / delta_f;
            const auto max_idx_dbl = static_cast<double>(num_bins - 1);

            for (size_t i = 0; i < num_bins; ++i) {
                const double erb_bins = 0.107939 * static_cast<double>(i) + const_term;
                const double half_width = erb_bins * factor;
                const double lower = std::max(0.0, static_cast<double>(i) - half_width);
                const double upper = std::min(max_idx_dbl, static_cast<double>(i) + half_width);

                low_idx_[i] = static_cast<size_t>(std::round(lower));
                high_idx_[i] = std::min(num_bins, static_cast<size_t>(std::round(upper) + 1.0));

                const size_t bin_count = high_idx_[i] - low_idx_[i];
                count_req_[i] = 1.0f / static_cast<float>(std::max<size_t>(1, bin_count));
            }
        }

        void smooth(const std::span<float> spectrum_abs_sqr) {
            assert(spectrum_abs_sqr.size() == low_idx_.size());
            applyBoxcarAverage(spectrum_abs_sqr);
            applyBoxcarAverage(spectrum_abs_sqr);
        }

    private:
        std::vector<size_t> low_idx_, high_idx_;
        std::vector<float> count_req_;
        std::vector<double> temp_cum_sum_;

        void applyBoxcarAverage(const std::span<float> data) {
            temp_cum_sum_[0] = 0.0;
            for (size_t i = 0; i < data.size(); ++i) {
                temp_cum_sum_[i + 1] = temp_cum_sum_[i] + static_cast<double>(data[i]);
            }
            for (size_t i = 0; i < data.size(); ++i) {
                data[i] = static_cast<float>(temp_cum_sum_[high_idx_[i]] - temp_cum_sum_[low_idx_[i]]) * count_req_[i];
            }
        }
    };
}
