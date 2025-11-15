// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_fft_panel.hpp"

namespace zlpanel {
    MatchFFTPanel::MatchFFTPanel(PluginProcessor& p,
                                 zlgui::UIBase& base,
                                 multilingual::TooltipHelper& tooltip_helper) :
        Thread("match_analyzer"),
        p_ref_(p), base_(base), tooltip_helper_(tooltip_helper),
        fft_min_db_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTMinDB::kID)),
        eq_max_db_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PEQMaxDB::kID)) {
        p_ref_.getController().getEQMatchAnalyzer().setMinFreq(10.0);
        base_.getPanelValueTree().addListener(this);
    }

    MatchFFTPanel::~MatchFFTPanel() {
        base_.getPanelValueTree().removeListener(this);
        stopThread(-1);
    }

    void MatchFFTPanel::paint(juce::Graphics& g) {
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        const auto thickness = base_.getFontSize() * .2f;
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPreColour).withAlpha(1.f));
        g.strokePath(source_path_, {thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded});
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kSideColour).withAlpha(1.f));
        g.strokePath(target_path_, {thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded});
        g.setColour(base_.getTextColour());
        g.strokePath(diff_path_, {thickness * 1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded});
    }

    void MatchFFTPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        if (bound.getHeight() < 1.f) {
            return;
        }
        bound.setWidth(bound.getWidth() * kFFTSizeOverWidth);
        updateSampleRate(sample_rate_);

        const auto bottom_area_height = getBottomAreaHeight(base_.getFontSize());
        const auto positive_height = base_.getFontSize() * kDraggerScale;
        const auto negative_height = bound.getHeight() - positive_height;
        const auto mid_height = negative_height - positive_height - static_cast<float>(bottom_area_height);

        min_ratio_.store(negative_height / mid_height, std::memory_order::relaxed);
        max_ratio_.store(-positive_height / mid_height, std::memory_order::relaxed);

        if (mid_height > 1.f) {
            drawing_k_ = -1.f / (mid_height * .5f);
            drawing_b_ = -(positive_height + mid_height * .5f);
            k_.store(1.f / drawing_k_, std::memory_order::relaxed);
            b_.store(-drawing_b_, std::memory_order::relaxed);
        }
    }

    void MatchFFTPanel::updateSampleRate(const double sample_rate) {
        if (sample_rate < 1.0) {
            return;
        }
        if (std::abs(sample_rate - sample_rate_) > 1.0) {
            sample_rate_ = sample_rate;
            p_ref_.getController().getEQMatchAnalyzer().setMaxFreq(sample_rate * .5 - 0.1);
        }
        const auto fft_max = freq_helper::getFFTMax(sample_rate);
        auto bound = getLocalBounds().toFloat();
        bound.setWidth(bound.getWidth() * kFFTSizeOverWidth);
        bound.setWidth(bound.getWidth() * static_cast<float>(
            std::log((sample_rate * .5 - 0.1) * 0.1) / std::log(fft_max * 0.1)));
        fft_width_ = bound.getWidth();
        atomic_bound_.store(bound);
    }

    void MatchFFTPanel::run() {
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
            auto& analyzer{p_ref_.getController().getEQMatchAnalyzer()};
            if (!analyzer.getLock().try_lock()) {
                continue;
            }
            const auto akima_reset_flag = analyzer.run();
            const size_t n = analyzer.getInterplotSize();
            if (akima_reset_flag || n != xs_.size()) {
                xs_.resize(n);
                source_ys_.resize(n);
                target_ys_.resize(n);
                diff_ys_.resize(n);
                c_width_ = -1.f;
            }
            analyzer.prepareDiff();
            if (std::abs(bound.getWidth() - c_width_) > 1e-3f) {
                c_width_ = bound.getWidth();
                analyzer.createPathXs(xs_, c_width_);
            }
            const auto fft_min_idx = fft_min_db_ref_.load(std::memory_order_relaxed);
            const auto fft_min = zlstate::PFFTMinDB::kDBs[static_cast<size_t>(std::round(fft_min_idx))];
            const auto max_db = max_ratio_.load(std::memory_order_relaxed) * fft_min;
            const auto min_db = min_ratio_.load(std::memory_order_relaxed) * fft_min;
            analyzer.createPathYs(std::span(source_ys_), std::span(target_ys_),
                                  bound.getHeight(), min_db, max_db);
            analyzer.prepareTarget();
            analyzer.prepareDrawing();
            analyzer.runDiff();
            const auto eq_max_idx = eq_max_db_ref_.load(std::memory_order_relaxed);
            const auto eq_max = zlstate::PEQMaxDB::kDBs[static_cast<size_t>(std::round(eq_max_idx))];
            analyzer.createDiffPathYs(diff_ys_, k_.load(std::memory_order_relaxed) / eq_max,
                                      b_.load(std::memory_order_relaxed));

            analyzer.getLock().unlock();
            if (xs_.empty()) {
                continue;
            }
            if (threadShouldExit()) {
                break;
            }
            updatePath(next_source_path_, bound, source_ys_);
            updatePath(next_target_path_, bound, target_ys_);
            updatePath(next_diff_path_, bound, diff_ys_);
            std::lock_guard<std::mutex> lock{mutex_};
            source_path_.swapWithPath(next_source_path_);
            target_path_.swapWithPath(next_target_path_);
            diff_path_.swapWithPath(next_diff_path_);
        }
    }

    void MatchFFTPanel::updatePath(juce::Path& path, const juce::Rectangle<float>& bound, std::span<float> ys) const {
        path.clear();
        path.startNewSubPath(xs_.front() - bound.getWidth() * 0.05f, ys.front());
        for (size_t i = 0; i < xs_.size(); ++i) {
            if (std::isfinite(ys[i])) {
                path.lineTo(xs_[i], ys[i]);
            }
        }
        path.lineTo(xs_.back(), ys.back());
    }

    void MatchFFTPanel::visibilityChanged() {
        if (isVisible()) {
            p_ref_.getController().getEQMatchAnalyzer().reset();
            p_ref_.getController().setEQMatchAnalyzerON(true);
            startThread(juce::Thread::Priority::low);
        } else {
            stopThread(-1);
            p_ref_.getController().getEQMatchAnalyzer().clearDrawingDiffs();
            p_ref_.getController().setEQMatchAnalyzerON(false);
            source_path_.clear();
            target_path_.clear();
            diff_path_.clear();
        }
    }

    void MatchFFTPanel::mouseDown(const juce::MouseEvent& event) {
        if (!draw_on_) {
            return;
        }
        const auto eq_max_idx = eq_max_db_ref_.load(std::memory_order_relaxed);
        const auto eq_max = zlstate::PEQMaxDB::kDBs[static_cast<size_t>(std::round(eq_max_idx))];
        drawing_actual_k_ = drawing_k_ * eq_max;

        pre_drawing_p_ = std::round(event.position.x / fft_width_ * 100.f) / 100.f;
        if (event.mods.isRightButtonDown()) {
            pre_drawing_db_ = drawing_actual_k_ * (event.position.y + drawing_b_);
        } else {
            pre_drawing_db_ = 0.f;
        }
    }

    void MatchFFTPanel::mouseDrag(const juce::MouseEvent& event) {
        if (!draw_on_) {
            return;
        }
        auto& analyzer{p_ref_.getController().getEQMatchAnalyzer()};
        const auto c_drawing_p = std::round(event.position.x / fft_width_ * 100.f) / 100.f;
        const auto count = static_cast<size_t>(std::round(std::abs(pre_drawing_p_ - c_drawing_p) / 0.01f));
        if (count == 0) {
            if (pre_drawing_p_ < 0.f || pre_drawing_p_ > 1.f) {
                return;
            }
            if (event.mods.isRightButtonDown()) {
                analyzer.clearDrawingDiffs(pre_drawing_p_);
            } else if (event.mods.isShiftDown()) {
                analyzer.setDrawingDiffs(pre_drawing_p_, 0.f);
                pre_drawing_db_ = 0.f;
            } else {
                const auto c_drawing_db = drawing_actual_k_ * (event.position.y + drawing_b_);
                analyzer.setDrawingDiffs(pre_drawing_p_, pre_drawing_db_);
                pre_drawing_db_ = c_drawing_db;
            }
        } else {
            const auto delta_p = c_drawing_p > pre_drawing_p_ ? 0.01f : -0.01f;
            if (event.mods.isRightButtonDown()) {
                for (size_t i = 0; i < count; ++i) {
                    if (pre_drawing_p_ >= 0.f && pre_drawing_p_ <= 1.f) {
                        analyzer.clearDrawingDiffs(pre_drawing_p_);
                    }
                    pre_drawing_p_ += delta_p;
                }
            } else if (event.mods.isShiftDown()) {
                for (size_t i = 0; i < count; ++i) {
                    if (pre_drawing_p_ >= 0.f && pre_drawing_p_ <= 1.f) {
                        analyzer.setDrawingDiffs(pre_drawing_p_, 0.f);
                    }
                    pre_drawing_p_ += delta_p;
                }
                pre_drawing_db_ = 0.f;
            } else {
                const auto c_drawing_db = drawing_actual_k_ * (event.position.y + drawing_b_);
                const auto delta_db = (c_drawing_db - pre_drawing_db_) / (static_cast<float>(count));
                for (size_t i = 0; i < count; ++i) {
                    if (pre_drawing_p_ >= 0.f && pre_drawing_p_ <= 1.f) {
                        analyzer.setDrawingDiffs(pre_drawing_p_, pre_drawing_db_);
                    }
                    pre_drawing_p_ += delta_p;
                    pre_drawing_db_ += delta_db;
                }
                pre_drawing_db_ = c_drawing_db;
            }
        }
        pre_drawing_p_ = c_drawing_p;
    }

    void MatchFFTPanel::mouseDoubleClick(const juce::MouseEvent& event) {
        if (!draw_on_) {
            return;
        }
        auto& analyzer{p_ref_.getController().getEQMatchAnalyzer()};
        if (event.mods.isLeftButtonDown()) {
            analyzer.clearDrawingDiffs();
        }
    }

    void MatchFFTPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kMatchDrawing, property)) {
            draw_on_ = static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kMatchDrawing)) > 0.5;
        } else if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kMatchPanel, property)) {
            const auto f = static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kMatchPanel));
            const auto idx = static_cast<int>(std::round(f));
            if (idx == 0) {
                setTooltip("");
            } else if (idx == 1) {
                setTooltip(tooltip_helper_.getToolTipText(multilingual::kEQMatchNotification1));
            } else if (idx == 2) {
                setTooltip(tooltip_helper_.getToolTipText(multilingual::kEQMatchNotification2));
            } else if (idx == 3) {
                setTooltip(tooltip_helper_.getToolTipText(multilingual::kEQMatchNotification3));
            }
        }
    }
}
