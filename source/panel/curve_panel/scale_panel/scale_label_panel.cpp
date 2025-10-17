// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "scale_label_panel.hpp"

namespace zlpanel {
    ScaleLabelPanel::ScaleLabelPanel(PluginProcessor& p,
                                     zlgui::UIBase& base,
                                     const multilingual::TooltipHelper& tooltip_helper) :
        base_(base) {
        juce::ignoreUnused(p, tooltip_helper);
        setInterceptsMouseClicks(false, false);
    }

    void ScaleLabelPanel::paint(juce::Graphics& g) {
        if (c_eq_max_idx_ < 0 || c_fft_min_idx_ < 0) {
            return;
        }
        // draw colour gradient background
        const auto bound = getLocalBounds().toFloat();
        juce::ColourGradient gradient;
        gradient.point1 = juce::Point<float>(bound.getX(), bound.getY());
        gradient.point2 = juce::Point<float>(bound.getRight(), bound.getY());
        gradient.isRadial = false;
        gradient.clearColours();
        gradient.addColour(0.0, base_.getBackgroundColor().withAlpha(0.f));
        gradient.addColour(1.0, base_.getBackgroundColor().withAlpha(1.f));
        g.setGradientFill(gradient);
        g.fillRect(getLocalBounds());
        // calculate values
        const auto unit_height = getUnitHeight();
        const float label_height = base_.getFontSize() * 1.1f;
        const float y0 = base_.getFontSize() * kDraggerScale - label_height * .5f;

        const auto eq_unit = zlstate::PEQMaxDB::kDBs[static_cast<size_t>(c_eq_max_idx_)] / 3.f;
        const auto fft_unit = zlstate::PFFTMinDB::kDBs[static_cast<size_t>(c_fft_min_idx_)] / 6.f;
        g.setFont(base_.getFontSize() * 1.25f);

        const auto db_label_bound = juce::Rectangle<float>(bound.getWidth() * .5f + base_.getFontSize() * .2f, y0,
                                                           bound.getWidth(), label_height);
        g.setColour(base_.getTextColor());
        g.drawText("dB", db_label_bound, juce::Justification::centredLeft);
        // draw eq labels and fft labels
        for (int i = 1; i < 7; ++i) {
            auto fft_label_bound = juce::Rectangle<float>(
                0.f, y0 + static_cast<float>(i) * unit_height, bound.getWidth(), label_height);
            fft_label_bound.removeFromRight(base_.getFontSize() * .1f);
            auto eq_label_bound = fft_label_bound.removeFromLeft(fft_label_bound.getWidth() * .5f);
            if (i != 6) {
                const auto fft_value = std::round(static_cast<float>(i) * fft_unit);
                g.setColour(base_.getTextColor().withAlpha(kFFTAlpha));
                g.drawText(juce::String(fft_value), fft_label_bound, juce::Justification::centredRight);
            }
            const auto eq_value = std::round((3.f - static_cast<float>(i)) * eq_unit);
            g.setColour(base_.getTextColor());
            g.drawText(juce::String(eq_value), eq_label_bound, juce::Justification::centredRight);
        }
        // draw the remaining fft labels
        for (int i = 7; i < 12; ++i) {
            auto fft_label_bound = juce::Rectangle<float>(
                0.f, y0 + static_cast<float>(i) * unit_height, bound.getWidth(), label_height);
            if (fft_label_bound.getBottom() > bound.getHeight() - base_.getFontSize() * 3.f) {
                break;
            }
            fft_label_bound.removeFromRight(base_.getFontSize() * .1f);
            const auto fft_value = std::round(static_cast<float>(i) * fft_unit);
            g.setColour(base_.getTextColor().withAlpha(kFFTAlpha));
            g.drawText(juce::String(fft_value), fft_label_bound, juce::Justification::centredRight);
        }
    }

    void ScaleLabelPanel::setMaxIdx(const int eq_max_idx, const int fft_min_idx) {
        if (eq_max_idx != c_eq_max_idx_ || fft_min_idx != c_fft_min_idx_) {
            c_eq_max_idx_ = eq_max_idx;
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
