// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "scale_label_panel.hpp"
#include "scale_panel_layout.hpp"

namespace zlpanel {
    ScaleLabelPanel::ScaleLabelPanel(PluginProcessor& p,
                                     zlgui::UIBase& base,
                                     const multilingual::TooltipHelper& tooltip_helper) :
        base_(base) {
        juce::ignoreUnused(p, tooltip_helper);
        setInterceptsMouseClicks(false, false);
    }

    void ScaleLabelPanel::paint(juce::Graphics& g) {
        if (c_eq_max_idx_ < 0 || c_fft_top_idx_ < 0 || c_fft_min_idx_ < 0) {
            return;
        }
        // draw colour gradient background
        const auto full_bound = getLocalBounds().toFloat();
        const auto fft_top = zlstate::PFFTTopDB::kDBs[static_cast<size_t>(c_fft_top_idx_)];
        const auto fft_range = zlstate::PFFTMinDB::kDBs[static_cast<size_t>(c_fft_min_idx_)];
        const auto use_wide_layout = scale_panel_layout::usesThreeDigitFloor(fft_top, fft_range);
        const auto layout = scale_panel_layout::getMetrics(
            full_bound.getWidth(), base_.getFontSize(), use_wide_layout);
        const auto bound = full_bound.withLeft(full_bound.getRight() - layout.content_width);
        juce::ColourGradient gradient;
        gradient.point1 = juce::Point<float>(bound.getX(), bound.getY());
        gradient.point2 = juce::Point<float>(bound.getRight(), bound.getY());
        gradient.isRadial = false;
        gradient.clearColours();
        gradient.addColour(0.0, base_.getBackgroundColour().withAlpha(0.f));
        gradient.addColour(1.0, base_.getBackgroundColour().withAlpha(1.f));
        g.setGradientFill(gradient);
        g.fillRect(bound);
        // calculate values
        const auto unit_height = getUnitHeight();
        const float label_height = base_.getFontSize() * 1.1f;
        const float y0 = base_.getFontSize() * kDraggerScale - label_height * .5f;

        const auto eq_unit = base_.getCurveDBScale(static_cast<size_t>(c_eq_max_idx_)) / 3.f;
        const auto fft_unit = fft_range / 6.f;
        g.setFont(base_.getFontSize() * 1.25f);

        // draw eq labels and fft labels
        for (int i = 1; i < 7; ++i) {
            auto fft_label_bound = juce::Rectangle<float>(
                bound.getX(), y0 + static_cast<float>(i) * unit_height, bound.getWidth(), label_height);
            const auto eq_label_bound = fft_label_bound.withWidth(layout.eq_column_width);
            if (use_wide_layout) {
                fft_label_bound.removeFromLeft(layout.eq_column_width);
            }
            fft_label_bound.removeFromRight(layout.right_padding);
            if (i != 6) {
                const auto fft_value = std::round(fft_top + static_cast<float>(i) * fft_unit);
                g.setColour(base_.getTextColour().withAlpha(kFFTAlpha));
                g.drawText(juce::String(fft_value), fft_label_bound,
                           juce::Justification::centredRight, false);
            }
            const auto eq_value = (3.f - static_cast<float>(i)) * eq_unit;
            if (std::abs(std::round(eq_value) - eq_value) < 0.01f) {
                g.setColour(base_.getTextColour());
                g.drawText(juce::String(static_cast<int>(std::round(eq_value))), eq_label_bound,
                           juce::Justification::centredRight, false);
            }
        }
        // draw the remaining fft labels
        for (int i = 7; i < 12; ++i) {
            auto fft_label_bound = juce::Rectangle<float>(
                bound.getX(), y0 + static_cast<float>(i) * unit_height, bound.getWidth(), label_height);
            if (fft_label_bound.getBottom() > full_bound.getHeight() - base_.getFontSize() * 3.f) {
                break;
            }
            if (use_wide_layout) {
                fft_label_bound.removeFromLeft(layout.eq_column_width);
            }
            fft_label_bound.removeFromRight(layout.right_padding);
            const auto fft_value = std::round(fft_top + static_cast<float>(i) * fft_unit);
            g.setColour(base_.getTextColour().withAlpha(kFFTAlpha));
            g.drawText(juce::String(fft_value), fft_label_bound, juce::Justification::centredRight);
        }
    }

    void ScaleLabelPanel::setScaleIdx(const int eq_max_idx, const int fft_top_idx, const int fft_min_idx) {
        if (eq_max_idx != c_eq_max_idx_ || fft_top_idx != c_fft_top_idx_ || fft_min_idx != c_fft_min_idx_) {
            c_eq_max_idx_ = eq_max_idx;
            c_fft_top_idx_ = fft_top_idx;
            c_fft_min_idx_ = fft_min_idx;
            repaint();
        }
    }

    float ScaleLabelPanel::getUnitHeight() const {
        const auto bound = getLocalBounds().toFloat();
        return (bound.getHeight() - 2.f * base_.getFontSize() * kDraggerScale
            - static_cast<float>(getBottomAreaHeight(base_.getFontSize()))) / 6.f;
    }
}
