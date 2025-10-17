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
                if (band != selected_band && !is_same_stereo_[band]) {
                    drawBand(g, band);
                }
            }
            for (const auto& band : not_off_indices_) {
                if (band != selected_band && is_same_stereo_[band]) {
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

        const auto band_colour = base_.getColorMap1(band);
        const auto background_color = base_.getBackgroundColor();
        base_stroke_colour_[band] = juce::Colour::fromFloatRGBA(
            multiplier * band_colour.getFloatRed() + (1 - multiplier) * background_color.getFloatRed(),
            multiplier * band_colour.getFloatGreen() + (1 - multiplier) * background_color.getFloatGreen(),
            multiplier * band_colour.getFloatBlue() + (1 - multiplier) * background_color.getFloatBlue(),
            1.f);

        is_same_stereo_[band] = is_same_stereo;
    }

    void SinglePanel::run(const size_t band,
                          const zlp::FilterStatus filter_status,
                          const bool to_update_base, const bool to_update_target,
                          std::span<float> xs, const float k, const float b,
                          kfr::univector<float>& base_mag, kfr::univector<float>& target_mag,
                          const zldsp::filter::FilterType filter_type,
                          const float center_x, const float center_mag) {
        const auto center_y = center_y_.load(std::memory_order_relaxed);
        if (to_update_base) {
            next_base_paths_[band].clear();
            next_base_fills_[band].clear();
            if (filter_status != zlp::FilterStatus::kOff) {
                temp_db_ = k * base_mag + b;
                // draw base path
                if (filter_type == zldsp::filter::kPeak || filter_type == zldsp::filter::kNotch) {
                    const auto it = std::upper_bound(xs.begin(), xs.end(), center_x);
                    if (it != xs.begin() && it != xs.end()) {
                        const auto index = static_cast<size_t>(std::distance(xs.begin(), it));
                        PathMinimizer minimizer{next_base_paths_[band]};
                        minimizer.startNewSubPath(xs[0], temp_db_[0]);
                        for (size_t i = 1; i < index; ++i) {
                            minimizer.lineTo(xs[i], temp_db_[i]);
                        }
                        minimizer.lineTo(center_x, k * center_mag + b);
                        for (size_t i = index; i < xs.size(); ++i) {
                            minimizer.lineTo(xs[i], temp_db_[i]);
                        }
                        minimizer.finish();
                    } else {
                        PathMinimizer minimizer{next_base_paths_[band]};
                        minimizer.drawPath<true, false>(xs, std::span(temp_db_));
                    }
                } else {
                    PathMinimizer minimizer{next_base_paths_[band]};
                    minimizer.drawPath<true, false>(xs, std::span(temp_db_));
                }
                // draw base fill
                next_base_fills_[band] = next_base_paths_[band];
                next_base_fills_[band].lineTo(xs.back(), center_y);
                next_base_fills_[band].lineTo(xs[0], center_y);
                next_base_fills_[band].closeSubPath();
                // draw button line
                if (filter_type == zldsp::filter::kLowPass
                    || filter_type == zldsp::filter::kHighPass
                    || filter_type == zldsp::filter::kNotch
                    || filter_type == zldsp::filter::kTiltShelf) {
                    next_button_lines_[band].setStart(center_x, k * center_mag + b);
                    next_button_lines_[band].setEnd(center_x, center_y);
                } else {
                    next_button_lines_[band].setEnd(-100.f, -100.f);
                }
            }
        }
        if (to_update_target) {
            next_target_fills_[band].clear();
            if (filter_status != zlp::FilterStatus::kOff) {
                temp_db_ = k * target_mag + b;
                // draw target fill
                next_target_fills_[band] = next_base_paths_[band];
                PathMinimizer minimizer{next_target_fills_[band]};
                minimizer.drawPath<false, true>(xs, std::span(temp_db_));
                next_target_fills_[band].closeSubPath();
            }
        }
    }

    void SinglePanel::runUpdate(std::array<bool, zlp::kBandNum>& to_update_base_flags,
                                std::array<bool, zlp::kBandNum>& to_update_target_flags) {
        std::lock_guard<std::mutex> lock{mutex_};
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (std::exchange(to_update_base_flags[band], false)) {
                base_paths_[band].swapWithPath(next_base_paths_[band]);
                base_fills_[band].swapWithPath(next_base_fills_[band]);
                button_lines_[band] = next_button_lines_[band];
            }
            if (std::exchange(to_update_target_flags[band], false)) {
                target_fills_[band].swapWithPath(next_target_fills_[band]);
            }
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
        if (base_stroke_alpha_[band] > 0.01f) {
            g.setColour(base_stroke_colour_[band]);
            g.strokePath(base_paths_[band], juce::PathStrokeType(curve_thickness_,
                                                                 juce::PathStrokeType::curved,
                                                                 juce::PathStrokeType::butt));
            if (button_lines_[band].getEndY() > 0.f) {
                g.drawLine(button_lines_[band], curve_thickness_ * .75f);
            }
        }
    }

    void SinglePanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .15f * base_.getEQCurveThickness();
    }
}
