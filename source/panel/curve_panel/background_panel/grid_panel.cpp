// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "grid_panel.hpp"

namespace zlpanel {
    GridPanel::GridPanel(zlgui::UIBase &base) : ui_base_(base) {
        setInterceptsMouseClicks(false, false);
    }

    GridPanel::~GridPanel() = default;

    void GridPanel::paint(juce::Graphics &g) {
        g.setFont(ui_base_.getFontSize() * zlgui::kFontLarge);
        if (ui_base_.getColourByIdx(zlgui::kGridColour).getFloatAlpha() <= 0.01f) {
            return;
        }
        g.setColour(ui_base_.getTextInactiveColor());
        for (size_t i = 0; i < kBackgroundFreqs.size(); ++i) {
            g.drawText(kBackgroundFreqsNames[i], text_bounds_[i], juce::Justification::bottomRight);
        }
        g.setColour(ui_base_.getColourByIdx(zlgui::kGridColour));
        g.fillRectList(rect_list_);
    }

    void GridPanel::resized() {
        rect_list_.clear();
        auto bound = getLocalBounds().toFloat();
        const auto thickness = ui_base_.getFontSize() * 0.1f;
        for (size_t i = 0; i < kBackgroundFreqs.size(); ++i) {
            const auto x = kBackgroundFreqs[i] * bound.getWidth() + bound.getX();
            rect_list_.add({x - thickness * .5f, bound.getY(), thickness, bound.getHeight()});
            text_bounds_[i] = juce::Rectangle<float>(x - ui_base_.getFontSize() * 3 - ui_base_.getFontSize() * 0.125f,
                                                   bound.getBottom() - ui_base_.getFontSize() * 2,
                                                   ui_base_.getFontSize() * 3, ui_base_.getFontSize() * 2);
        }

        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * ui_base_.getFontSize());

        for (auto &d: kBackgroundDBs) {
            const auto y = d * bound.getHeight() + bound.getY();
            rect_list_.add({bound.getX(), y - thickness * .5f, bound.getWidth(), thickness});
        }
    }
}
