// Copyright (C) 2026 - zsliu98
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

    void SinglePanel::paintSameStereo(juce::Graphics& g) {
        const auto selected_band = base_.getSelectedBand();
        if (selected_band < zlp::kBandNum) {
            for (const auto& band : not_off_indices_) {
                if (band != selected_band && is_same_stereo_[band]) {
                    drawBand<false>(g, band);
                }
            }
            drawBand<true>(g, selected_band);
            if (target_fill_alpha_[selected_band] > 0.01f) {
                g.setColour(base_stroke_colour_[selected_band]);
                side_lines_[selected_band].pull();
                const auto line = side_lines_[selected_band].getReader();
                g.fillRect(line.getStartX(), side_y_ - curve_thickness_ * .35f,
                           line.getEndX() - line.getStartX(), curve_thickness_ * .7f);
            }
        }
    }

    void SinglePanel::paintDifferentStereo(juce::Graphics& g) {
        const auto selected_band = base_.getSelectedBand();
        if (selected_band < zlp::kBandNum) {
            for (const auto& band : not_off_indices_) {
                if (band != selected_band && !is_same_stereo_[band]) {
                    drawBand<false>(g, band);
                }
            }
        } else {
            for (const auto& band : not_off_indices_) {
                drawBand<false>(g, band);
            }
        }
    }

    void SinglePanel::resized() {
        auto bound = getLocalBounds().toFloat();
        bound.removeFromBottom(static_cast<float>(getBottomAreaHeight(base_.getFontSize())));
        center_y_.store(bound.getHeight() * .5f, std::memory_order::relaxed);
        side_y_ = bound.getHeight() - base_.getFontSize() * kDraggerScale * .5f;
        lookAndFeelChanged();
    }

    void SinglePanel::updateDrawingParas(const size_t band,
                                         const zlp::FilterStatus filter_status,
                                         const bool is_dynamic_on, const bool is_same_stereo) {
        if (filter_status == zlp::FilterStatus::kOff) {
            base_fill_alpha_[band] = 0.f;
            target_fill_alpha_[band] = 0.f;
            base_stroke_alpha_[band] = 0.f;
            return;
        }
        float multiplier = 1.f;
        const auto selected_band = base_.getSelectedBand();
        const auto is_selected = (band == selected_band);
        if (filter_status == zlp::FilterStatus::kBypass) {
            multiplier *= kBypassAlphaMultiplier;
        }
        if (!is_same_stereo) {
            multiplier *= kDiffStereoAlphaMultiplier;
        } else if (selected_band == zlp::kBandNum) {
            multiplier *= kNoBandSelectedAlphaMultiplier;
        }
        if (is_selected) {
            base_fill_alpha_[band] = (is_dynamic_on ? 0.f : kFillingAlpha) * multiplier;
            target_fill_alpha_[band] = (is_dynamic_on ? kDynamicFillingAlpha : 0.f) * multiplier;
        } else {
            target_fill_alpha_[band] = (is_dynamic_on ? kDynamicFillingAlpha * .9f : 0.f) * multiplier;
            multiplier *= kNotSelectedAlphaMultiplier;
            base_fill_alpha_[band] = 0.f;
        }
        base_stroke_alpha_[band] = multiplier;

        const auto band_colour = base_.getColourMap1(band);
        base_stroke_colour_[band] = base_.getColourBlendedWithBackground(band_colour, multiplier);

        is_same_stereo_[band] = is_same_stereo;
    }

    void SinglePanel::run(const size_t band,
                          const zlp::FilterStatus filter_status,
                          const bool to_update_base, const bool to_update_target,
                          std::span<float> xs, const float k, const float b,
                          zldsp::vector::aligned_vector<float>& base_mag,
                          zldsp::vector::aligned_vector<float>& target_mag,
                          const float center_x, const float center_mag, const float button_mag,
                          const float base_left_x, const float base_right_x,
                          const bool to_update_side, const float left_x, const float right_x,
                          const bool is_all_pass, const bool is_first_order) {
        const auto center_y = center_y_.load(std::memory_order_relaxed);
        auto& next_base_path{base_paths_[band].getWriter()};
        auto& next_base_fill{base_fills_[band].getWriter()};
        auto& next_button_line{button_lines_[band].getWriter()};
        auto& next_target_fill{target_fills_[band].getWriter()};
        auto& next_side_line{side_lines_[band].getWriter()};
        auto& next_all_pass_line{all_pass_lines_[band].getWriter()};

        if (to_update_base) {
            next_base_path.clear();
            next_base_fill.clear();
            next_button_line.setEnd(-100.f, -100.f);
            next_all_pass_line.setStart(-100.f, 0.f);
            next_all_pass_line.setEnd(-100.f, 0.f);
            if (filter_status != zlp::FilterStatus::kOff) {
                zldsp::vector::fma(temp_db_.data(), base_mag.data(), k, b, temp_db_.size());
                if (is_all_pass) {
                    next_base_path.startNewSubPath(center_x, 0.f);
                    next_base_path.lineTo(center_x, center_y * 2.f);
                    if (!is_first_order) {
                        next_all_pass_line.setStart(base_left_x, 0.f);
                        next_all_pass_line.setEnd(base_right_x, 0.f);
                    }
                } else {
                    // draw base path
                    PathMinimizer<1> minimizer{next_base_path};
                    minimizer.drawPath<true, false>(xs, std::span(temp_db_));
                    // draw base fill
                    next_base_fill = next_base_path;
                    next_base_fill.lineTo(xs.back(), center_y);
                    next_base_fill.lineTo(xs[0], center_y);
                    next_base_fill.closeSubPath();
                }
                // draw button line
                if (!is_all_pass && std::abs(center_mag - button_mag) > 1e-6f) {
                    next_button_line.setStart(center_x, center_mag);
                    next_button_line.setEnd(center_x, button_mag);
                }
            }
        }
        if (to_update_target) {
            next_target_fill.clear();
            if (filter_status != zlp::FilterStatus::kOff && !is_all_pass) {
                zldsp::vector::fma(temp_db_.data(), target_mag.data(), k, b, temp_db_.size());
                // draw target fill
                next_target_fill = next_base_path;
                PathMinimizer<1> minimizer{next_target_fill};
                minimizer.drawPath<false, true>(xs, std::span(temp_db_));
                next_target_fill.closeSubPath();
            }
        }
        if (to_update_side) {
            next_side_line.setStart(left_x, 0.f);
            next_side_line.setEnd(right_x, 0.f);
        }
        if (to_update_base) {
            base_paths_[band].publish();
            base_fills_[band].publish();
            button_lines_[band].publish();
            all_pass_lines_[band].publish();
        }
        if (to_update_target) {
            target_fills_[band].publish();
        }
        if (to_update_side) {
            side_lines_[band].publish();
        }
    }

    template <bool thick>
    void SinglePanel::drawBand(juce::Graphics& g, const size_t band) {
        const auto colour = base_.getColourMap1(band);
        if (base_fill_alpha_[band] > 0.01f) {
            g.setColour(colour.withAlpha(base_fill_alpha_[band]));
            base_fills_[band].pull();
            g.fillPath(base_fills_[band].getReader());
        }
        if (target_fill_alpha_[band] > 0.01f) {
            g.setColour(colour.withAlpha(target_fill_alpha_[band]));
            target_fills_[band].pull();
            g.fillPath(target_fills_[band].getReader());
        }
        const auto curve_thickness = thick ? curve_thickness_ * kThickMultiplier : curve_thickness_;
        if (base_stroke_alpha_[band] > 0.01f) {
            g.setColour(base_stroke_colour_[band]);
            base_paths_[band].pull();
            g.strokePath(base_paths_[band].getReader(), juce::PathStrokeType(curve_thickness,
                                                                             juce::PathStrokeType::curved,
                                                                             juce::PathStrokeType::square));
            button_lines_[band].pull();
            if (const auto line = button_lines_[band].getReader(); line.getEndX() > 0.f) {
                if (line.getEndY() > line.getStartY()) {
                    g.fillRect(line.getStartX() - curve_thickness * .35f, line.getStartY(),
                               curve_thickness * .7f, line.getEndY() - line.getStartY());
                } else {
                    g.fillRect(line.getStartX() - curve_thickness * .35f, line.getEndY(),
                               curve_thickness * .7f, line.getStartY() - line.getEndY());
                }
            }
            if (thick) {
                all_pass_lines_[band].pull();
                const auto ap_line = all_pass_lines_[band].getReader();
                if (ap_line.getEndX() > 0.f || ap_line.getEndY() > 0.f) {
                    const auto center_y = center_y_.load(std::memory_order_relaxed);
                    g.fillRect(ap_line.getStartX() - curve_thickness * .35f, center_y * .5f,
                               curve_thickness * .7f, center_y);
                    g.fillRect(ap_line.getEndX() - curve_thickness * .35f, center_y * .5f,
                               curve_thickness * .7f, center_y);
                }
            }
        }
    }

    void SinglePanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .175f * base_.getSingleEQCurveThickness();
    }
}
