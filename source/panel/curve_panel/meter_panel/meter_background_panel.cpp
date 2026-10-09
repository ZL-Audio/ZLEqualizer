// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "meter_background_panel.hpp"

#include "../../helper/panel_constants.hpp"

namespace zlpanel {
    MeterBackgroundPanel::MeterBackgroundPanel(PluginProcessor&, zlgui::UIBase& base) :
        base_(base) {
        setInterceptsMouseClicks(false, false);
    }

    void MeterBackgroundPanel::paint(juce::Graphics& g) {
        auto bound = getLocalBounds().toFloat();
        g.fillAll(base_.getBackgroundColour());
        const auto font_size = base_.getFontSize();
        bound.removeFromTop(static_cast<float>(getTopPanelHeight(font_size)));

        const auto thickness = font_size * .1f;
        const auto padding = font_size * kDraggerScale;
        const auto unit_height = (bound.getHeight() - 2.f * padding
            - static_cast<float>(getBottomAreaHeight(font_size))) / 6.f;
        if (unit_height <= 0.f) {
            return;
        }
        g.setColour(base_.getTextColour().withAlpha(.1f));
        for (auto y = bound.getY() + padding - thickness * .5f;
             y + thickness < bound.getBottom() - font_size * 3.f; y += unit_height) {
            const auto rect = juce::Rectangle<float>({bound.getX(), y, bound.getWidth(), thickness});
            g.fillRect(rect);
        }
    }
}
