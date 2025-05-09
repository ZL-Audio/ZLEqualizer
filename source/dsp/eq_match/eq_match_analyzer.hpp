// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../fft_analyzer/fft_analyzer.hpp"

namespace zldsp::eq_match {
    template<typename FloatType>
    class EqMatchAnalyzer final : private juce::Thread {
    public:
        enum MatchMode {
            kMatchSide,
            kMatchPreset,
            kMatchSlope
        };

        static constexpr size_t kPointNum = 251, kSmoothSize = 11;
        static constexpr float kAvgDB = -36.f;

        explicit EqMatchAnalyzer(size_t fft_order = 12);

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<FloatType> &b_buffer, juce::AudioBuffer<FloatType> &t_buffer);

        zldsp::analyzer::AverageFFTAnalyzer<FloatType, 2, kPointNum> &getAverageFFT() { return fft_analyzer_; }

        void setON(bool x);

        void reset();

        void checkRun();

        void setMatchMode(MatchMode mode) {
            mode_.store(mode);
        }

        void setTargetSlope(float x);

        void setTargetPreset(const std::array<float, kPointNum> &dbs);

        void updatePaths(juce::Path &mainP, juce::Path &targetP, juce::Path &diffP,
                         juce::Rectangle<float> bound, std::array<float, 3> minDBs);

        void setSmooth(const float x) {
            smooth.store(x);
            to_update_smooth_.store(true);
        }

        void setSlope(const float x) { slope.store(x); }

        void setShift(const float x) { shift.store(x); }

        std::array<std::atomic<float>, kPointNum> &getTarget() { return atomic_target_dbs_; }

        std::array<std::atomic<float>, kPointNum> &getDiffs() { return atomic_diffs_; }

        void setDrawingDiffs(const size_t idx, const float x) {
            drawing_diffs_[idx].store(x - shift.load());
            drawing_flag_[idx].store(true);
        }

        void clearDrawingDiffs(const size_t idx) {
            drawing_flag_[idx].store(false);
        }

        void clearDrawingDiffs() {
            for (size_t idx = 0; idx < kPointNum; ++idx) {
                drawing_flag_[idx].store(false);
            }
        }

    private:
        zldsp::analyzer::AverageFFTAnalyzer<FloatType, 2, kPointNum> fft_analyzer_;
        std::array<float, kPointNum> main_dbs_{}, target_dbs_{}, diffs_{};
        std::array<std::atomic<float>, kPointNum> atomic_target_dbs_{}, atomic_diffs_{};
        std::array<std::atomic<bool>, kPointNum> drawing_flag_;
        std::array<std::atomic<float>, kPointNum> drawing_diffs_;
        std::atomic<MatchMode> mode_;
        std::atomic<bool> is_on_{false};
        std::array<float, kPointNum> load_dbs_{};
        std::atomic<bool> to_update_from_load_{false};

        std::atomic<float> smooth{.5f}, slope{.0f}, shift{0.f};
        std::atomic<bool> to_update_smooth_{true};
        std::array<float, kSmoothSize> smooth_kernel_{};
        float rescale_ = 1.f;
        std::array<float, kPointNum + kSmoothSize - 1> original_diffs_{};

        void run() override;

        void updateSmooth() {
            if (to_update_smooth_.exchange(false)) {
                smooth_kernel_[kSmoothSize / 2] = 1.0;
                const auto currentSmooth = std::clamp(smooth.load(), 0.f, .5f);
                rescale_ = std::clamp(2.f - 2 * smooth.load(), 0.f, 1.f);
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
                for (auto &x: smooth_kernel_) {
                    x *= kernelC;
                }
            }
        }
    };
} // zldsp::eq_match
