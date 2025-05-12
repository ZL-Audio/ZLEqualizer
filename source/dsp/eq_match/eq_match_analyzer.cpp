// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "eq_match_analyzer.hpp"

namespace zldsp::eq_match {
    template<typename FloatType>
    EqMatchAnalyzer<FloatType>::EqMatchAnalyzer(const size_t fft_order)
        : Thread("eq_match_analyzer"), fft_analyzer_(fft_order) {
        std::fill(main_dbs_.begin(), main_dbs_.end(), 0.f);
        std::fill(target_dbs_.begin(), target_dbs_.end(), 0.f);
        std::fill(diffs_.begin(), diffs_.end(), 0.f);
        updateSmooth();
        clearDrawingDiffs();
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        fft_analyzer_.prepare(spec);
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::process(juce::AudioBuffer<FloatType> &b_buffer,
                                             juce::AudioBuffer<FloatType> &t_buffer) {
        if (is_on_.load()) {
            fft_analyzer_.process({b_buffer, t_buffer});
        }
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::setON(const bool x) {
        if (x != is_on_.load()) {
            if (x) {
                fft_analyzer_.setON(0, true);
                fft_analyzer_.setON(1, true);
                is_on_.store(true);
                if (!isThreadRunning()) {
                    startThread(Priority::low);
                }
            } else {
                is_on_.store(false);
                fft_analyzer_.setON(0, false);
                fft_analyzer_.setON(1, false);
                if (isThreadRunning()) {
                    stopThread(-1);
                }
            }
        }
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::reset() {
        fft_analyzer_.reset();
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::run() {
        juce::ScopedNoDenormals no_denormals;
        while (!threadShouldExit()) {
            fft_analyzer_.run();
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
        }
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::checkRun() {
        notify();
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::setTargetSlope(const float x) {
        const float tilt_shift_total = (fft_analyzer_.kMaxFreqLog2 - fft_analyzer_.kMinFreqLog2) * x;
        const float tilt_shift_delta = tilt_shift_total / static_cast<float>(kPointNum - 1);
        float tilt_shift = -tilt_shift_total * .5f;
        if (to_update_from_load_.load() == false) {
            for (size_t i = 0; i < load_dbs_.size(); i++) {
                load_dbs_[i] = tilt_shift;
                tilt_shift += tilt_shift_delta;
            }
            to_update_from_load_.store(true);
        }
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::setTargetPreset(const std::array<float, kPointNum> &dbs) {
        if (to_update_from_load_.load() == false) {
            for (size_t i = 0; i < dbs.size(); i++) {
                load_dbs_[i] = dbs[i];
            }
            to_update_from_load_.store(true);
        }
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::updatePaths(juce::Path &mainP, juce::Path &targetP, juce::Path &diffP,
                                                 const juce::Rectangle<float> bound,
                                                 const std::array<float, 3> minDBs) {
        // update mainDBs and targetDBs
        {
            main_dbs_ = fft_analyzer_.getInterplotDBs(0);
            const auto max_db = *std::max_element(main_dbs_.begin(), main_dbs_.end());
            for (auto &dB: main_dbs_) {
                dB -= max_db;
            }
        }
        if (mode_.load() == MatchMode::kMatchSide) {
            target_dbs_ = fft_analyzer_.getInterplotDBs(1);
            const auto max_db = *std::max_element(target_dbs_.begin(), target_dbs_.end());
            for (auto &dB: target_dbs_) {
                dB -= max_db;
            }
        } else if (to_update_from_load_.load()) {
            target_dbs_ = load_dbs_;
            to_update_from_load_.store(false);
        }
        // calculate original diffs
        for (size_t i = 0; i < kPointNum; ++i) {
            original_diffs_[i + smooth_kernel_.size() / 2] = target_dbs_[i] - main_dbs_[i];
        }
        // pad original diffs
        for (size_t i = 0; i < smooth_kernel_.size() / 2; ++i) {
            original_diffs_[i] = original_diffs_[smooth_kernel_.size() / 2];
        }
        for (size_t i = kPointNum + smooth_kernel_.size() / 2; i < original_diffs_.size(); ++i) {
            original_diffs_[i] = original_diffs_[kPointNum + smooth_kernel_.size() / 2 - 1];
        }
        // update smooth and slope
        updateSmooth();
        // apply smooth
        std::fill(diffs_.begin(), diffs_.end(), 0.f);
        for (size_t i = 0; i < kPointNum; ++i) {
            for (size_t j = 0; j < smooth_kernel_.size(); ++j) {
                diffs_[i] += original_diffs_[i + j] * smooth_kernel_[j];
            }
        }
        // apply slope
        const auto slope_total = (fft_analyzer_.kMaxFreqLog2 - fft_analyzer_.kMinFreqLog2) * slope.load();
        const float slope_delta = slope_total / static_cast<float>(kPointNum - 1);
        float slope_shift = -slope_total * .5f;
        for (size_t i = 0; i < kPointNum; ++i) {
            diffs_[i] += slope_shift;
            slope_shift += slope_delta;
        }
        // scale diffs
        if (rescale_ < .99f) {
            for (auto &diff: diffs_) {
                diff *= rescale_;
            }
        }
        // center diffs
        const auto current_shift = shift.load();
        const auto diff_c = std::reduce(
            diffs_.begin(), diffs_.end(), 0.f) / static_cast<float>(diffs_.size()) - current_shift;
        for (size_t i = 0; i < kPointNum; ++i) {
            diffs_[i] = drawing_flag_[i].load() ? drawing_diffs_[i].load() + current_shift : diffs_[i] - diff_c;
        }
        // save to target
        for (size_t i = 0; i < kPointNum; ++i) {
            atomic_target_dbs_[i].store(target_dbs_[i]);
            atomic_diffs_[i].store(diffs_[i]);
        }
        // create paths
        const float width = bound.getWidth(), height = bound.getHeight(), bound_y = bound.getY();
        const std::array<juce::Path *, 3> paths{&mainP, &targetP, &diffP};
        for (const auto &p: paths) {
            p->clear();
        }
        const std::array<std::array<float, kPointNum> *, 3> dbs{&main_dbs_, &target_dbs_, &diffs_};
        for (size_t i = 0; i < 3; ++i) {
            const auto y = (dbs[i]->at(0) / minDBs[i] + .5f) * height + bound_y;
            paths[i]->startNewSubPath(0.f, y);
        }
        for (size_t idx = 1; idx < kPointNum; ++idx) {
            const auto x = static_cast<float>(idx) / static_cast<float>(kPointNum - 1) * width;
            for (size_t i = 0; i < 3; ++i) {
                const auto y = (dbs[i]->at(idx) / minDBs[i] + .5f) * height + bound_y;
                paths[i]->lineTo(x, y);
            }
        }
    }

    template
    class EqMatchAnalyzer<float>;

    template
    class EqMatchAnalyzer<double>;
} // zldsp::eq_match
