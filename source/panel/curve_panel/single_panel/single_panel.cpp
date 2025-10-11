// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "single_panel.hpp"

namespace zlpanel {
    SinglePanel::SinglePanel(PluginProcessor& p,
                             zlgui::UIBase& base,
                             std::vector<size_t>& not_off_indices) :
        p_ref_(p), base_(base),
        not_off_indices_(not_off_indices) {
        juce::ignoreUnused(p_ref_);
        setInterceptsMouseClicks(false, false);
    }

    void SinglePanel::paint(juce::Graphics& g) {
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        const auto selected_band = base_.getSelectedBand();
        if (selected_band < zlp::kBandNum) {
            drawBand(g, selected_band);
            for (const auto& band : not_off_indices_) {
                if (band != selected_band) {
                    drawBand(g, band);
                }
            }
        } else {
            for (const auto& band : not_off_indices_) {
                drawBand(g, band);
            }
        }
    }

    void SinglePanel::resized() {
        bound_.store(getLocalBounds().toFloat());
        lookAndFeelChanged();
    }

    void SinglePanel::updateDrawingParas(const size_t band, const zlp::FilterStatus filter_status,
                                         const bool is_dynamic_on, const bool is_same_stereo) {
        if (filter_status == zlp::FilterStatus::kOff) {
            base_fill_alpha_[band] = 0.f;
            target_fill_alpha_[band] = 0.f;
            base_stroke_alpha_[band] = 0.f;
            target_stroke_alpha_[band] = 0.f;
            return;
        }
        float multiplier = 1.f;
        const auto is_selected = (band == base_.getSelectedBand());
        if (filter_status == zlp::FilterStatus::kBypass) {
            multiplier *= kBypassAlphaMultiplier;
        }
        if (!is_selected) {
            multiplier *= kNotSelectedAlphaMultiplier;
        }
        if (!is_same_stereo) {
            multiplier *= kDiffStereoAlphaMultiplier;
        }
        if (!is_selected) {
            multiplier *= kNotSelectedAlphaMultiplier;
        }
        if (is_selected) {
            base_fill_alpha_[band] = (is_dynamic_on ? 0.f : kFillingAlpha) * multiplier;
            target_stroke_alpha_[band] = (is_dynamic_on ? 1.f : 0.f) * multiplier;
        } else {
            base_fill_alpha_[band] = 0.f;
            target_stroke_alpha_[band] = 0.f;
        }
        target_fill_alpha_[band] = (is_dynamic_on ? kDynamicFillingAlpha : 0.f) * multiplier;
        base_stroke_alpha_[band] = multiplier;
    }

    void SinglePanel::run(std::span<size_t> to_update_indices,
                          std::span<float> xs,
                          std::array<std::span<float>, zlp::kBandNum> base_yss,
                          std::array<std::span<float>, zlp::kBandNum> target_yss) {
        const auto bound = bound_.load();
        for (const auto& band : to_update_indices) {
            // clear all paths and fills
            next_base_paths_[band].clear();
            next_base_fills_[band].clear();
            next_target_paths_[band].clear();
            next_target_fills_[band].clear();
            if (!base_yss[band].empty()) {
                const auto ys = base_yss[band];
                // draw base path
                PathMinimizer minimizer{next_base_paths_[band]};
                minimizer.drawPath<true, false>(xs, ys);
                // draw base fill
                next_base_fills_[band] = next_base_paths_[band];
                next_base_fills_[band].lineTo(xs.back(), bound.getCentreY());
                next_base_fills_[band].lineTo(xs[0], bound.getCentreY());
                next_base_fills_[band].closeSubPath();
            }

            if (!base_yss[band].empty() && !target_yss[band].empty()) {
                const auto ys = target_yss[band];
                // draw target path
                PathMinimizer minimizer1{next_target_paths_[band]};
                minimizer1.drawPath<true, false>(xs, ys);
                // draw target fill
                next_target_fills_[band] = next_base_paths_[band];
                PathMinimizer minimizer2{next_target_fills_[band]};
                minimizer2.drawPath<false, true>(xs, ys);
                next_target_fills_[band].closeSubPath();
            }
        }
        std::lock_guard<std::mutex> lock{mutex_};
        for (const auto& band : to_update_indices) {
            base_paths_[band].swapWithPath(next_base_paths_[band]);
            base_fills_[band].swapWithPath(next_base_fills_[band]);
            target_paths_[band].swapWithPath(next_target_paths_[band]);
            target_fills_[band].swapWithPath(next_target_fills_[band]);
        }
    }

    void SinglePanel::drawBand(juce::Graphics& g, const size_t band) const {
        const auto colour = base_.getColorMap1(band);
        if (base_fill_alpha_[band] > 0.01f) {
            g.setColour(colour.withAlpha(base_fill_alpha_[band]));
            g.fillPath(base_fills_[band]);
        }
        if (target_fill_alpha_[band] > 0.01f) {
            g.setColour(colour.withAlpha(target_fill_alpha_[band]));
            g.fillPath(target_fills_[band]);
        }
        if (target_stroke_alpha_[band] > 0.01f) {
            g.setColour(colour.withAlpha(target_stroke_alpha_[band]));
            g.strokePath(target_paths_[band], juce::PathStrokeType(curve_thickness_,
                                                                   juce::PathStrokeType::curved,
                                                                   juce::PathStrokeType::rounded));
        }
        if (base_stroke_alpha_[band] > 0.01f) {
            g.setColour(colour.withAlpha(base_stroke_alpha_[band]));
            g.strokePath(base_paths_[band], juce::PathStrokeType(curve_thickness_,
                                                                 juce::PathStrokeType::curved,
                                                                 juce::PathStrokeType::rounded));
        }
    }

    void SinglePanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .15f * base_.getEQCurveThickness();
    }
}
