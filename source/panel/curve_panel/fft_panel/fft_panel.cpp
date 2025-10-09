// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fft_panel.hpp"

namespace zlpanel {
    FFTPanel::FFTPanel(PluginProcessor& p,
                       zlgui::UIBase& base,
                       const multilingual::TooltipHelper& tooltip_helper) :
        Thread("fft_panel"),
        p_ref_(p), base_(base),
        pre_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTPreON::kID)),
        post_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTPostON::kID)),
        side_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTSideON::kID)),
        fft_min_db_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTMinDB::kID)) {
        juce::ignoreUnused(tooltip_helper);
        p_ref_.getController().getFFTAnalyzer().setMinFreq(10.0);

        setInterceptsMouseClicks(false, false);
    }

    FFTPanel::~FFTPanel() {
        stopThread(-1);
    }

    void FFTPanel::paint(juce::Graphics& g) {
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        // return;
        const auto pre_on = pre_ref_.load(std::memory_order::relaxed) > .5f;
        const auto post_on = post_ref_.load(std::memory_order::relaxed) > .5f;
        const auto side_on = side_ref_.load(std::memory_order::relaxed) > .5f;

        if (pre_on) {
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPreColour));
            g.fillPath(pre_path_);
        } else {
            pre_path_.clear();
        }
        if (post_on) {
            const auto thickness = base_.getFontSize() * .2f;
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPostColour).withAlpha(1.f));
            g.strokePath(post_path_, {thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded});
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPostColour));
            g.fillPath(post_path_);
        } else {
            post_path_.clear();
        }
        if (side_on) {
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kSideColour));
            g.fillPath(side_path_);
        } else {
            side_path_.clear();
        }
    }

    void FFTPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        if (bound.getHeight() < 1.f) {
            return;
        }
        bound.removeFromRight(base_.getFontSize() * kDraggerScale);
        updateSampleRate(sample_rate_);

        const auto bottom_area_height = getBottomAreaHeight(base_.getFontSize());
        const auto positive_height = base_.getFontSize() * kDraggerScale;
        const auto negative_height = bound.getHeight() - positive_height;
        const auto mid_height = negative_height - positive_height * 2.f - static_cast<float>(bottom_area_height);

        min_ratio_.store(negative_height / mid_height, std::memory_order::relaxed);
        max_ratio_.store(-positive_height / mid_height, std::memory_order::relaxed);
    }

    void FFTPanel::updateSampleRate(const double sample_rate) {
        if (sample_rate < 1.0) {
            return;
        }
        if (std::abs(sample_rate - sample_rate_) > 1.0) {
            sample_rate_ = sample_rate;
            p_ref_.getController().getFFTAnalyzer().setMaxFreq(sample_rate * .5 - 0.1);
        }
        const auto fft_max = freq_helper::getFFTMax(sample_rate);
        auto bound = getLocalBounds().toFloat();
        bound.removeFromRight(base_.getFontSize() * kDraggerScale);
        bound.setWidth(bound.getWidth() * static_cast<float>(
            std::log((sample_rate * .5 - 0.1) * 0.1) / std::log(fft_max * 0.1)));
        atomic_bound_.store(bound);
    }

    void FFTPanel::run() {
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            if (threadShouldExit()) {
                break;
            }
            const auto bound = atomic_bound_.load();
            if (bound.getWidth() < .1f) {
                continue;
            }
            auto& analyzer{p_ref_.getController().getFFTAnalyzer()};
            if (!analyzer.getLock().try_lock()) {
                continue;
            }
            const auto akima_reset_flag = analyzer.run();
            const size_t n = analyzer.getInterplotSize();
            if (akima_reset_flag || n != xs_.size()) {
                xs_.resize(n);
                pre_ys_.resize(n);
                post_ys_.resize(n);
                side_ys_.resize(n);
                c_width_ = -1.f;
            }

            if (std::abs(bound.getWidth() - c_width_) > 1e-3f) {
                c_width_ = bound.getWidth();
                analyzer.createPathXs(xs_, c_width_);
            }
            const auto fft_min_idx = fft_min_db_ref_.load(std::memory_order_relaxed);
            const auto fft_min = zlstate::PFFTMinDB::kDBs[static_cast<size_t>(std::round(fft_min_idx))];
            const auto max_db = max_ratio_.load(std::memory_order_relaxed) * fft_min;
            const auto min_db = min_ratio_.load(std::memory_order_relaxed) * fft_min;
            analyzer.createPathYs({std::span(pre_ys_), std::span(post_ys_), std::span(side_ys_)},
                                  bound.getHeight(), min_db, max_db);
            analyzer.getLock().unlock();
            if (xs_.empty()) {
                continue;
            }
            if (threadShouldExit()) {
                break;
            }
            const auto pre_on = pre_ref_.load(std::memory_order::relaxed) > .5f;
            const auto post_on = post_ref_.load(std::memory_order::relaxed) > .5f;
            const auto side_on = side_ref_.load(std::memory_order::relaxed) > .5f;
            // update pre path
            if (pre_on) {
                updatePath(next_pre_path_, bound, pre_ys_);
            }
            // update post path
            if (post_on) {
                updatePath(next_post_path_, bound, post_ys_);
            }
            // update side path
            if (side_on) {
                updatePath(next_side_path_, bound, side_ys_);
            }
            std::lock_guard<std::mutex> lock{mutex_};
            if (pre_on) {
                pre_path_.swapWithPath(next_pre_path_);
            }
            if (post_on) {
                post_path_.swapWithPath(next_post_path_);
            }
            if (side_on) {
                side_path_.swapWithPath(next_side_path_);
            }
        }
    }

    void FFTPanel::updatePath(juce::Path& path, const juce::Rectangle<float>& bound, std::span<float> ys) const {
        path.clear();
        path.startNewSubPath(xs_[0] - .1f, bound.getBottom() + 10.f);
        for (size_t i = 0; i < xs_.size(); ++i) {
            if (std::isfinite(ys[i])) {
                path.lineTo(xs_[i], ys[i]);
            }
        }
        path.lineTo(bound.getRight() + .1f, bound.getBottom() + 10.f);
        path.closeSubPath();
    }
}
