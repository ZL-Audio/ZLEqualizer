// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "background_panel.hpp"

namespace zlpanel {
    BackgroundPanel::BackgroundPanel(PluginProcessor& p,
                                     zlgui::UIBase& base,
                                     const multilingual::TooltipHelper& tooltip_helper) :
        base_(base) {
        juce::ignoreUnused(p, tooltip_helper);
        setInterceptsMouseClicks(false, false);
        lookAndFeelChanged();
    }

    void BackgroundPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColour());
        if (freq_max_ < 10000.0) {
            return;
        }
        drawFreqs(g);
        drawDBs(g);
    }

    void BackgroundPanel::updateSampleRate(const double sample_rate) {
        if (sample_rate < 40000.0) {
            return;
        }
        freq_max_ = freq_helper::getFFTMax(sample_rate);
        repaint();
    }

    void BackgroundPanel::drawFreqs(juce::Graphics& g) const {
        auto bound = getLocalBounds().toFloat();
        const auto full_width = bound.getWidth();
        bound.setWidth(bound.getWidth() * kFFTSizeOverWidth);
        // draw freq grid
        const auto thickness = base_.getFontSize() * 0.1f;
        juce::RectangleList<float> rect_list;
        for (const auto& freq : kFreqValues) {
            const auto p = std::log(static_cast<double>(freq) * .1) / std::log(freq_max_ * .1);
            const auto rect = juce::Rectangle(static_cast<float>(p) * bound.getWidth() - thickness * .5f, 0.f,
                                              thickness, bound.getHeight());
            if (rect.getRight() > full_width) {
                break;
            }
            rect_list.add(rect);
        }
        g.setColour(grid_colour_);
        g.fillRectList(rect_list);
        // draw top and bottom gradient
        juce::ColourGradient gradient;
        gradient.point1 = juce::Point<float>(bound.getX(), bound.getY());
        gradient.point2 = juce::Point<float>(bound.getX(), bound.getBottom());
        gradient.isRadial = false;
        gradient.clearColours();
        gradient.addColour(0.0, base_.getBackgroundColour().withAlpha(1.f));
        gradient.addColour(base_.getFontSize() / bound.getHeight(),
                           base_.getBackgroundColour().withAlpha(0.f));
        gradient.addColour(1.f - 2.f * base_.getFontSize() / bound.getHeight(),
                           base_.getBackgroundColour().withAlpha(0.f));
        gradient.addColour(1.f - base_.getFontSize() / bound.getHeight(),
                           base_.getBackgroundColour().withAlpha(1.f));
        gradient.addColour(1.0, base_.getBackgroundColour().withAlpha(1.f));
        g.setGradientFill(gradient);
        g.fillRect(getLocalBounds());
        // draw freq labels
        g.setColour(base_.getTextColour().withAlpha(.5f));
        g.setFont(base_.getFontSize() * 1.25f);
        const auto label_y0 = bound.getBottom() - base_.getFontSize() * 1.15f;
        const auto label_height = base_.getFontSize() * 1.1f;
        for (const auto& freq : kFreqValues) {
            const auto label = freq < 1000.f ? juce::String(freq) : juce::String(std::round(freq * 0.001f)) + "K";
            const auto label_width = freq < 20000.f
                ? base_.getFontSize() * 3.f
                : juce::GlyphArrangement::getStringWidth(g.getCurrentFont(), label) * 1.1f;
            const auto p = std::log(static_cast<double>(freq) * .1) / std::log(freq_max_ * .1);
            const auto rect = juce::Rectangle(static_cast<float>(p) * bound.getWidth() - label_width * .5f, label_y0,
                                              label_width, label_height);
            if (rect.getRight() > full_width) {
                break;
            }
            g.drawText(freq < 1000.f ? juce::String(freq) : juce::String(std::round(freq * 0.001f)) + "K",
                       rect, juce::Justification::centredBottom, false);
        }
    }

    void BackgroundPanel::drawDBs(juce::Graphics& g) const {
        const auto bound = getLocalBounds().toFloat();
        const auto thickness = base_.getFontSize() * 0.1f;
        auto y0 = base_.getFontSize() - thickness * .5f;
        const auto unit_height = (bound.getHeight() - 2.f * base_.getFontSize() * kDraggerScale
            - static_cast<float>(getBottomAreaHeight(base_.getFontSize()))) / 6.f;

        juce::RectangleList<float> rect_list;
        while (y0 + thickness < bound.getHeight() - base_.getFontSize() * 3.f) {
            rect_list.add(0.f, y0, bound.getWidth(), thickness);
            y0 += unit_height;
        }
        g.setColour(grid_colour_);
        g.fillRectList(rect_list);
    }

    void BackgroundPanel::lookAndFeelChanged() {
        const auto grid_colour = base_.getColourByIdx(zlgui::ColourIdx::kGridColour);
        const auto background_colour = base_.getBackgroundColour();
        const auto alpha = grid_colour.getFloatAlpha();
        grid_colour_ = juce::Colour::fromFloatRGBA(
            grid_colour.getFloatRed() * alpha + background_colour.getFloatRed() * (1.f - alpha),
            grid_colour.getFloatGreen() * alpha + background_colour.getFloatGreen() * (1.f - alpha),
            grid_colour.getFloatBlue() * alpha + background_colour.getFloatBlue() * (1.f - alpha),
            1.f);
    }
}
