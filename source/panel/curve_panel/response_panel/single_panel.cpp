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
        temp_db_.resize(kNumPoints);
        setInterceptsMouseClicks(false, false);
    }

    void SinglePanel::paint(juce::Graphics& g) {
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        const auto selected_band = base_.getSelectedBand();
        if (selected_band < zlp::kBandNum) {
            for (const auto& band : not_off_indices_) {
                if (band != selected_band) {
                    drawBand(g, band);
                }
            }
            drawBand(g, selected_band);
        } else {
            for (const auto& band : not_off_indices_) {
                drawBand(g, band);
            }
        }
    }

    void SinglePanel::resized() {
        auto bound = getLocalBounds().toFloat();
        bound.removeFromBottom(static_cast<float>(getBottomAreaHeight(base_.getFontSize())));
        center_y_.store(bound.getHeight() * .5f, std::memory_order::relaxed);
        lookAndFeelChanged();
    }

    void SinglePanel::updateDrawingParas(const size_t band, const zlp::FilterStatus filter_status,
                                         const bool is_dynamic_on, const bool is_same_stereo) {
        if (filter_status == zlp::FilterStatus::kOff) {
            base_fill_alpha_[band] = 0.f;
            target_fill_alpha_[band] = 0.f;
            base_stroke_alpha_[band] = 0.f;
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
            target_fill_alpha_[band] = (is_dynamic_on ? kDynamicFillingAlpha : 0.f) * multiplier;
        } else {
            base_fill_alpha_[band] = 0.f;
            target_fill_alpha_[band] = (is_dynamic_on ? kFillingAlpha : 0.f) * multiplier;
        }
        base_stroke_alpha_[band] = multiplier;
    }

    bool SinglePanel::run(const juce::Thread& thread,
                          const std::array<zlp::FilterStatus, zlp::kBandNum>& filter_status,
                          std::array<bool, zlp::kBandNum>& to_update_base_flags,
                          std::array<bool, zlp::kBandNum>& to_update_target_flags,
                          const std::span<float> xs,
                          const float k, const float b,
                          std::array<kfr::univector<float>, zlp::kBandNum>& base_mags,
                          std::array<kfr::univector<float>, zlp::kBandNum>& target_mags) {
        const auto center_y = center_y_.load(std::memory_order_relaxed);
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            // clear all paths and fills
            if (to_update_base_flags[band]) {
                next_base_paths_[band].clear();
                next_base_fills_[band].clear();
                if (filter_status[band] != zlp::FilterStatus::kOff) {
                    temp_db_ = k * base_mags[band] + b;
                    // draw base path
                    PathMinimizer minimizer{next_base_paths_[band]};
                    minimizer.drawPath<true, false>(xs, std::span(temp_db_));
                    // draw base fill
                    next_base_fills_[band] = next_base_paths_[band];
                    next_base_fills_[band].lineTo(xs.back(), center_y);
                    next_base_fills_[band].lineTo(xs[0], center_y);
                    next_base_fills_[band].closeSubPath();
                }
                if (thread.threadShouldExit()) {
                    return false;
                }
            }
            if (to_update_target_flags[band]) {
                next_target_fills_[band].clear();
                if (filter_status[band] != zlp::FilterStatus::kOff) {
                    temp_db_ = k * target_mags[band] + b;
                    // draw target fill
                    next_target_fills_[band] = next_base_paths_[band];
                    PathMinimizer minimizer{next_target_fills_[band]};
                    minimizer.drawPath<false, true>(xs, std::span(temp_db_));
                    next_target_fills_[band].closeSubPath();
                }
                if (thread.threadShouldExit()) {
                    return false;
                }
            }
        }
        std::lock_guard<std::mutex> lock{mutex_};
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (std::exchange(to_update_base_flags[band], false)) {
                base_paths_[band].swapWithPath(next_base_paths_[band]);
                base_fills_[band].swapWithPath(next_base_fills_[band]);
            }
            if (std::exchange(to_update_target_flags[band], false)) {
                target_fills_[band].swapWithPath(next_target_fills_[band]);
            }
        }
        return true;
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
        if (base_stroke_alpha_[band] > 0.01f) {
            g.setColour(colour.withAlpha(base_stroke_alpha_[band]));
            g.strokePath(base_paths_[band], juce::PathStrokeType(curve_thickness_,
                                                                 juce::PathStrokeType::curved,
                                                                 juce::PathStrokeType::butt));
        }
    }

    void SinglePanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .15f * base_.getEQCurveThickness();
    }
}
