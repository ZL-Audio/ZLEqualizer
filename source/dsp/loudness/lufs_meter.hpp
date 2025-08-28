// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "k_weighting_filter.hpp"

namespace zldsp::loudness {
    template<typename FloatType, bool UseLowPass = false>
    class LUFSMeter {
    public:
        LUFSMeter() = default;

        void prepare(const double sample_rate, const size_t num_channels) {
            k_weighting_filter_.prepare(sample_rate, num_channels);
            weights_.resize(num_channels);
            for (size_t i = 0; i < num_channels; ++i) {
                weights_[i] = (i == 4 || i == 5) ? FloatType(1.41) : FloatType(1);
            }

            max_idx_ = static_cast<int>(sample_rate * 0.1);
            mean_mul_ = static_cast<FloatType>(2.5 / sample_rate);

            small_buffer_.resize(num_channels);
            small_buffer_ptrs_.resize(num_channels);
            for (size_t channel = 0; channel < num_channels; ++channel) {
                small_buffer_[channel].resize(static_cast<size_t>(max_idx_));
                small_buffer_ptrs_[channel] = small_buffer_[channel].data();
            }
            reset();
        }

        void reset() {
            k_weighting_filter_.reset();
            current_idx_ = 0;
            ready_count_ = 0;
            std::fill(histogram_.begin(), histogram_.end(), FloatType(0));
            std::fill(histogram_sums_.begin(), histogram_sums_.end(), FloatType(0));
            for (auto &buffer : small_buffer_) {
                std::fill(buffer.begin(), buffer.end(), FloatType(0));
            }
        }

        void process(std::span<FloatType *> buffer, const size_t num_samples) {
            const auto num_total = static_cast<int>(num_samples);
            int start_idx = 0;
            while (num_total - start_idx >= max_idx_ - current_idx_) {
                // now we get a full 100 ms small block
                const auto remaining_num = max_idx_ - current_idx_;
                for (size_t channel = 0; channel < buffer.size(); ++channel) {
                    auto input_v = kfr::make_univector(
                        buffer[channel] + static_cast<size_t>(start_idx),
                        static_cast<size_t>(remaining_num));
                    auto sub_v = kfr::make_univector(
                        small_buffer_[channel].data() + static_cast<size_t>(current_idx_),
                        static_cast<size_t>(remaining_num));
                    sub_v = input_v;
                }
                start_idx += remaining_num;
                current_idx_ = 0;
                update();
            }
            if (num_total - start_idx > 0) {
                const auto remaining_num = num_total - start_idx;
                for (size_t channel = 0; channel < buffer.size(); ++channel) {
                    auto input_v = kfr::make_univector(
                        buffer[channel] + static_cast<size_t>(start_idx),
                        static_cast<size_t>(remaining_num));
                    auto sub_v = kfr::make_univector(
                        small_buffer_[channel].data() + static_cast<size_t>(current_idx_),
                        static_cast<size_t>(remaining_num));
                    sub_v = input_v;
                }
                current_idx_ += remaining_num;
            }
        }

        FloatType getIntegratedLoudness() const {
            const auto total_count = kfr::sum(histogram_);
            if (total_count < FloatType(0.5)) { return FloatType(0); }
            const auto total_sum = kfr::sum(histogram_sums_);
            const auto total_mean_square = total_sum / total_count;
            const auto total_lufs = FloatType(-0.691) + FloatType(10) * std::log10(total_mean_square);
            if (total_lufs <= FloatType(-60)) {
                return total_lufs;
            } else {
                const auto end_idx = static_cast<size_t>(
                    std::round(-(total_lufs - FloatType(10)) * FloatType(10)));
                const auto sub_count = kfr::sum(histogram_.slice(0, end_idx));
                const auto sub_sum = kfr::sum(histogram_sums_.slice(0, end_idx));
                const auto sub_mean_square = sub_sum / sub_count;
                const auto sub_lufs = FloatType(-0.691) + FloatType(10) * std::log10(sub_mean_square);
                return sub_lufs;
            }
        }

    private:
        KWeightingFilter<FloatType, UseLowPass> k_weighting_filter_;
        std::vector<std::vector<FloatType>> small_buffer_;
        std::vector<FloatType *> small_buffer_ptrs_;
        int current_idx_{0}, max_idx_{0};
        int ready_count_{0};
        FloatType mean_mul_{1};
        std::array<FloatType, 4> sum_squares_{};

        kfr::univector<FloatType, 701> histogram_{};
        kfr::univector<FloatType, 701> histogram_sums_{};
        std::vector<FloatType> weights_;

        void update() {
            // perform K-weighting filtering
            k_weighting_filter_.process(std::span(small_buffer_ptrs_), static_cast<size_t>(max_idx_));
            // calculate the sum square of the small block
            FloatType sum_square = 0;
            for (size_t channel = 0; channel < small_buffer_.size(); ++channel) {
                auto channel_v = kfr::make_univector(small_buffer_[channel]);
                FloatType channel_sum_square = kfr::sumsqr(channel_v);
                sum_square += channel_sum_square * weights_[channel];
            }
            // shift circular sumSquares
            sum_squares_[0] = sum_squares_[1];
            sum_squares_[1] = sum_squares_[2];
            sum_squares_[2] = sum_squares_[3];
            sum_squares_[3] = sum_square;
            if (ready_count_ < 3) {
                ready_count_ += 1;
                return;
            }
            // calculate the mean square
            const auto mean_square = (sum_squares_[0] + sum_squares_[1] + sum_squares_[2] + sum_squares_[3]) * mean_mul_;
            // update histogram
            if (mean_square >= FloatType(1.1724653045822963e-7)) {
                // if greater than -70 LKFS
                const auto lkfs = std::min(-FloatType(0.691) + FloatType(10) * std::log10(mean_square), FloatType(0));
                const auto hist_idx = static_cast<size_t>(std::round(-lkfs * FloatType(10)));
                histogram_[hist_idx] += FloatType(1);
                histogram_sums_[hist_idx] += mean_square;
            }
        }
    };
}
