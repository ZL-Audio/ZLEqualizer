// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../fft_analyzer/multiple_avg_fft_base.hpp"

namespace zldsp::eq_match {
    template <typename FloatType, size_t kPointNum>
    class EqMatchAnalyzer final : public zldsp::analyzer::MultipleAvgFFTBase<FloatType, 2, kPointNum> {
    public:
        enum MatchMode {
            kMatchSide,
            kMatchPreset,
            kMatchSlope
        };

        static constexpr size_t kSmoothSize = 11;

        explicit EqMatchAnalyzer(size_t fft_order = 12) :
            zldsp::analyzer::MultipleAvgFFTBase<FloatType, 2, 251>(fft_order) {

        }

        // call this before starting the background thread
        void reset() {
            zldsp::analyzer::MultipleAvgFFTBase<FloatType, 2, 251>::reset();
            to_reset_match_.store(true, std::memory_order::release);
        }

        void setMatchMode(const MatchMode mode) {
            mode_.store(mode);
        }

        void setTargetSlope(const float x) {
            target_slope_.store(x);
            to_update_from_slope_.store(true);
        }

        void setTargetPreset(const std::span<float> freqs, const std::span<float> dbs) {
            if (!to_update_from_preset_.load()) {
                preset_freqs_.resize(freqs.size());
                preset_dbs_.resize(dbs.size());
                zldsp::vector::copy(preset_freqs_.data(), freqs.data(), preset_freqs_.size());
                zldsp::vector::copy(preset_dbs_.data(), dbs.data(), preset_dbs_.size());
                to_update_from_preset_.store(true);
            }
        }

        void prepareDiff() {
            if (to_reset_match_.exchange(false, std::memory_order::acquire)) {
                const auto n = this->getResultDBs()[0].size();
                diffs_.resize(n);
                original_diffs_.resize(n);
                saved_freqs_.resize(n);
                saved_target_dbs_.resize(n);
                saved_diffs_.resize(n);
                drawing_diff_indicator_.resize(n);
                drawing_diff_values_.resize(n);
                interpolated_target_dbs_.resize(n);

                const auto& ps{this->interplot_freqs_p_};
                size_t ps_idx = 0;
                drawing_mapping_start_[0] = 0;
                for (size_t i = 1; i < 101; ++i) {
                    const auto left_p = static_cast<float>(i) * .01f - 0.005f;
                    while (ps_idx < ps.size() && ps[ps_idx] < left_p) {
                        ps_idx += 1;
                    }
                    drawing_mapping_start_[i] = ps_idx;
                }

                std::lock_guard<std::mutex> lock(saved_mutex_);
                zldsp::vector::copy(saved_freqs_.data(), this->interplot_freqs_.data(), saved_freqs_.size());
            }
        }

        void prepareTarget() {
            c_mode_ = mode_.load();
            if (c_mode_ == kMatchSlope && to_update_from_slope_.exchange(false, std::memory_order::acquire)) {
                // update target from a slope value
                const auto min_log2 = std::log2(this->interplot_freqs_.front());
                const auto max_log2 = std::log2(this->interplot_freqs_.back());
                const auto total_slope = target_slope_.load(std::memory_order::relaxed) * (max_log2 - min_log2);
                const auto delta_slope = total_slope / static_cast<float>(this->interplot_freqs_.size() - 1);
                auto slope = total_slope * .5f;
                for (size_t i = 0; i < interpolated_target_dbs_.size(); ++i) {
                    interpolated_target_dbs_[i] = slope;
                    slope += delta_slope;
                }
            } else if (c_mode_ == kMatchPreset && to_update_from_preset_.load()) {
                // update target from a preset
                if (std::abs(preset_freqs_.back() - this->interplot_freqs_.back()) < 0.01f
                    && preset_dbs_.size() == interpolated_target_dbs_.size()) {
                    // if the freq values match exactly, copy db values
                    zldsp::vector::copy(interpolated_target_dbs_.data(), preset_dbs_.data(),
                                        interpolated_target_dbs_.size());
                } else {
                    // else interpolate the db values
                    auto interpolator = zldsp::interpolation::SeqMakima<float>(
                        preset_freqs_.data(), preset_dbs_.data(), preset_freqs_.size(), 0.f, 0.f);
                    interpolator.prepare();
                    interpolator.eval(this->interplot_freqs_.data(), interpolated_target_dbs_.data(),
                                      interpolated_target_dbs_.size());
                }
                to_update_from_preset_.store(false);
            }
        }

        void prepareDrawing() {
            if (to_update_drawing_.exchange(false, std::memory_order::acquire)) {
                for (size_t i = 0; i < 100; ++i) {
                    const auto f1 = drawing_flag_[i].load(std::memory_order::relaxed);
                    const auto v1 = drawing_diffs_[i].load(std::memory_order::relaxed);
                    for (size_t j = drawing_mapping_start_[i]; j < drawing_mapping_start_[i + 1]; ++j) {
                        drawing_diff_indicator_[j] = f1;
                        drawing_diff_values_[j] = v1;
                    }
                }
                const auto f1 = drawing_flag_.back().load(std::memory_order::relaxed);
                const auto v1 = drawing_diffs_.back().load(std::memory_order::relaxed);
                for (size_t j = drawing_mapping_start_.back(); j < drawing_diff_indicator_.size(); ++j) {
                    drawing_diff_indicator_[j] = f1;
                    drawing_diff_values_[j] = v1;
                }
            }
        }

        void runDiff() {
            // calculate diffs
            auto source_v = kfr::make_univector(this->getResultDBs()[0]);
            auto target_v = c_mode_ == kMatchSide
                ? kfr::make_univector(this->getResultDBs()[1])
                : kfr::make_univector(interpolated_target_dbs_);
            auto diff_v = kfr::make_univector(diffs_);
            diff_v = target_v - source_v;
            // center diffs & apply shift/drawing
            const auto c_shift = diff_shift_.load(std::memory_order::relaxed);
            const auto diff_c = kfr::mean(diff_v) - c_shift;
            for (size_t idx = 0; idx < diffs_.size(); ++idx) {
                diffs_[idx] = drawing_diff_indicator_[idx]
                    ? drawing_diff_values_[idx] + c_shift
                    : diffs_[idx] - diff_c;
            }
            // save target dbs & diffs
            std::lock_guard<std::mutex> lock(saved_mutex_);
            if (c_mode_ == kMatchSide) {
                zldsp::vector::copy(saved_target_dbs_.data(), target_v.data(), saved_target_dbs_.size());
            } else {
                zldsp::vector::copy(saved_target_dbs_.data(), interpolated_target_dbs_.data(),
                                    saved_target_dbs_.size());
            }
            zldsp::vector::copy(saved_diffs_.data(), diffs_.data(), saved_diffs_.size());
        }

        void createPathXs(std::span<float> xs, const float width) {
            for (size_t idx = 0; idx < this->interplot_freqs_p_.size(); ++idx) {
                xs[idx] = this->interplot_freqs_p_[idx] * width;
            }
        }

        void createPathYs(std::span<float> source_ys, std::span<float> target_ys, const float height,
                          const float min_db = -72.f, const float max_db = 0.f) {
            const auto scale = height / min_db;
            if (!source_ys.empty()) {
                auto db = kfr::make_univector(this->result_dbs_[0]);
                auto y = kfr::make_univector(source_ys);
                if (std::abs(max_db) > 0.01f) {
                    y = (db - max_db) * scale;
                } else {
                    y = db * scale;
                }
            }
            if (!target_ys.empty()) {
                auto target_db = c_mode_ == kMatchSide
                    ? kfr::make_univector(this->getResultDBs()[1])
                    : kfr::make_univector(interpolated_target_dbs_);
                auto y = kfr::make_univector(target_ys);
                if (std::abs(max_db) > 0.01f) {
                    y = (target_db - max_db) * scale;
                } else {
                    y = target_db * scale;
                }
            }
        }

        void createDiffPathYs(std::span<float> diff_ys, const float k, const float b) {
            if (!diffs_.empty()) {
                auto db = kfr::make_univector(diffs_);
                auto y = kfr::make_univector(diff_ys);
                y = db * k + b;
            }
        }

        void setDiffSmooth(const float x) {
            diff_smooth_.store(x);
            to_update_smooth_.store(true);
        }

        void setDiffSlope(const float x) {
            diff_slope_.store(x);
        }

        void setDiffShift(const float x) {
            diff_shift_.store(x);
        }

        void saveTarget(std::vector<float>& target_dbs) {
            std::lock_guard<std::mutex> lock(saved_mutex_);
            target_dbs.resize(saved_target_dbs_.size());
            zldsp::vector::copy(target_dbs.data(), saved_target_dbs_.data(), target_dbs.size());
        }

        void saveDiffs(std::vector<float>& diffs) {
            std::lock_guard<std::mutex> lock(saved_mutex_);
            diffs.resize(saved_diffs_.size());
            zldsp::vector::copy(diffs.data(), saved_diffs_.data(), diffs.size());
        }

        void setDrawingDiffs(const float idx_p, const float diff) {
            const auto idx = static_cast<size_t>(std::round(idx_p) * 100.f);
            drawing_diffs_[idx].store(diff - diff_shift_.load(std::memory_order::relaxed), std::memory_order::relaxed);
            drawing_flag_[idx].store(true, std::memory_order::relaxed);
            to_update_drawing_.store(true, std::memory_order::release);
        }

        void clearDrawingDiffs(const float idx_p) {
            const auto idx = static_cast<size_t>(std::round(idx_p) * 100.f);
            drawing_flag_[idx].store(false, std::memory_order::relaxed);
            to_update_drawing_.store(true, std::memory_order::release);
        }

        void clearDrawingDiffs() {
            for (size_t idx = 0; idx < drawing_flag_.size(); ++idx) {
                drawing_flag_[idx].store(false, std::memory_order::relaxed);
            }
            to_update_drawing_.store(true, std::memory_order::release);
        }

    private:
        MatchMode c_mode_{MatchMode::kMatchSide};
        std::atomic<MatchMode> mode_{MatchMode::kMatchSide};
        std::atomic<bool> to_reset_match_{false};

        std::vector<float> diffs_{};
        std::vector<float> original_diffs_{};
        std::vector<float> saved_freqs_{};
        std::vector<float> saved_target_dbs_{};
        std::vector<float> saved_diffs_{};
        std::mutex saved_mutex_;

        std::array<std::atomic<bool>, 101> drawing_flag_;
        std::array<std::atomic<float>, 101> drawing_diffs_;
        std::array<size_t, 101> drawing_mapping_start_{};
        std::atomic<bool> to_update_drawing_{false};
        std::vector<bool> drawing_diff_indicator_{};
        std::vector<float> drawing_diff_values_{};

        std::atomic<float> target_slope_{4.5f};
        std::atomic<bool> to_update_from_slope_{false};
        std::vector<float> preset_freqs_{}, preset_dbs_{};
        std::atomic<bool> to_update_from_preset_{false};
        std::vector<float> interpolated_target_dbs_{};

        std::atomic<float> diff_smooth_{.5f}, diff_slope_{.0f}, diff_shift_{0.f};
        std::atomic<bool> to_update_smooth_{true};
        std::array<float, kSmoothSize> smooth_kernel_{};
        float rescale_ = 1.f;

        void updateSmooth() {
            if (to_update_smooth_.exchange(false)) {
                smooth_kernel_[kSmoothSize / 2] = 1.0;
                const auto currentSmooth = std::clamp(diff_smooth_.load(), 0.f, .5f);
                rescale_ = std::clamp(2.f - 2 * diff_smooth_.load(), 0.f, 1.f);
                constexpr float midSlope = -1.f / static_cast<float>(kSmoothSize / 2);
                const auto tempSlope = currentSmooth < 0.5
                    ? -1.f * (1 - currentSmooth * 2.f) + currentSmooth * 2.f * midSlope
                    : (2.f - 2.f * currentSmooth) * midSlope;
                for (size_t i = 1; i < kSmoothSize / 2 + 1; i++) {
                    smooth_kernel_[kSmoothSize / 2 + i] = std::max(tempSlope * static_cast<float>(i) + 1, 0.f);
                    smooth_kernel_[kSmoothSize / 2 - i] = smooth_kernel_[kSmoothSize / 2 + i];
                }
                const auto kernelC = 1.f /
                    std::max(std::reduce(smooth_kernel_.begin(), smooth_kernel_.end(), 0.0f), 0.01f);
                for (auto& x : smooth_kernel_) {
                    x *= kernelC;
                }
            }
        }
    };
}
