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
    template<typename FloatType, size_t MaxChannels = 2, bool UseLowPass = false>
    class LUFSMeter {
    public:
        LUFSMeter() {
            for (size_t i = 0; i < MaxChannels; ++i) {
                weights_[i] = (i == 4 || i == 5) ? FloatType(1.41) : FloatType(1);
            }
        }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            k_weighting_filter_.prepare(spec);
            max_idx_ = static_cast<int>(spec.sampleRate * 0.1);
            mean_mul_ = static_cast<FloatType>(2.5 / spec.sampleRate);
            small_buffer_.setSize(static_cast<int>(spec.numChannels), max_idx_);
            reset();
        }

        void reset() {
            k_weighting_filter_.reset();
            std::fill(histogram_.begin(), histogram_.end(), FloatType(0));
            std::fill(histogram_sums_.begin(), histogram_sums_.end(), FloatType(0));
            current_idx_ = 0;
            small_buffer_.clear();
            ready_count_ = 0;
        }

        void process(juce::AudioBuffer<FloatType> &buffer) {
            process(juce::dsp::AudioBlock<FloatType>(buffer));
        }

        void process(juce::dsp::AudioBlock<FloatType> block) {
            const auto num_total = static_cast<int>(block.getNumSamples());
            int start_idx = 0;
            juce::dsp::AudioBlock<FloatType> small_block(small_buffer_);
            while (num_total - start_idx >= max_idx_ - current_idx_) {
                // now we get a full 100 ms small block
                const auto sub_block = block.getSubBlock(static_cast<size_t>(start_idx),
                                                        static_cast<size_t>(max_idx_ - current_idx_));
                auto small_sub_block = small_block.getSubBlock(static_cast<size_t>(current_idx_),
                                                            static_cast<size_t>(max_idx_ - current_idx_));
                small_sub_block.copyFrom(sub_block);
                start_idx += max_idx_ - current_idx_;
                current_idx_ = 0;
                update();
            }
            if (num_total - start_idx > 0) {
                const auto sub_block = block.getSubBlock(static_cast<size_t>(start_idx),
                                                        static_cast<size_t>(num_total - start_idx));
                auto small_sub_block = small_block.getSubBlock(static_cast<size_t>(current_idx_),
                                                            static_cast<size_t>(num_total - start_idx));
                small_sub_block.copyFrom(sub_block);
                current_idx_ += num_total - start_idx;
            }
        }

        FloatType getIntegratedLoudness() const {
            const auto total_count = std::reduce(histogram_.begin(), histogram_.end(), FloatType(0));
            if (total_count < FloatType(0.5)) { return FloatType(0); }
            const auto total_sum = std::reduce(histogram_sums_.begin(), histogram_sums_.end(), FloatType(0));
            const auto total_mean_square = total_sum / total_count;
            const auto total_lufs = FloatType(-0.691) + FloatType(10) * std::log10(total_mean_square);
            if (total_lufs <= FloatType(-60)) {
                return total_lufs;
            } else {
                const auto end_idx = static_cast<typename std::array<FloatType, 701>::difference_type>(
                    std::round(-(total_lufs - FloatType(10)) * FloatType(10)));
                const auto sub_count = std::reduce(histogram_.begin(), histogram_.begin() + end_idx, FloatType(0));
                const auto sub_sum = std::reduce(histogram_sums_.begin(), histogram_sums_.begin() + end_idx, FloatType(0));
                const auto sub_mean_square = sub_sum / sub_count;
                const auto sub_lufs = FloatType(-0.691) + FloatType(10) * std::log10(sub_mean_square);
                return sub_lufs;
            }
        }

    private:
        KWeightingFilter<FloatType, UseLowPass> k_weighting_filter_;
        juce::AudioBuffer<FloatType> small_buffer_;
        int current_idx_{0}, max_idx_{0};
        int ready_count_{0};
        FloatType mean_mul_{1};
        std::array<FloatType, 4> sum_squares_{};

        std::array<FloatType, 701> histogram_{};
        std::array<FloatType, 701> histogram_sums_{};
        std::array<FloatType, MaxChannels> weights_;

        void update() {
            // perform K-weighting filtering
            k_weighting_filter_.process(small_buffer_);
            // calculate the sum square of the small block
            FloatType sum_square = 0;
            for (int channel = 0; channel < small_buffer_.getNumChannels(); ++channel) {
                const auto reader_pointer = small_buffer_.getReadPointer(channel);
                FloatType channel_sum_square = 0;
                for (int i = 0; i < small_buffer_.getNumSamples(); ++i) {
                    const auto sample = *(reader_pointer + i);
                    channel_sum_square += sample * sample;
                }
                sum_square += channel_sum_square * weights_[static_cast<size_t>(channel)];
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
