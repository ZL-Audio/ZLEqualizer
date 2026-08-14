// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "panel_surface_background.hpp"

#include <utility>

#include "popup_style.hpp"

namespace zlgui::popup {
    PanelSurfaceBackground::PanelSurfaceBackground(UIBase& base, const float alpha) : base_(base) {
        setInterceptsMouseClicks(false, false);
        setAlpha(alpha);
    }

    void PanelSurfaceBackground::paint(juce::Graphics& g) {
        const auto padding = paddingSize(base_.getFontSize());
        const auto bound = getLocalBounds().reduced(padding);
        juce::Path path;
        path.addRoundedRectangle(bound.toFloat(), static_cast<float>(padding));

        const juce::DropShadow shadow{base_.getTextColour().withAlpha(.5f), padding, {0, 0}};
        shadow.drawForPath(g, path);
        g.setColour(surfaceColour(base_.getBackgroundColour(), base_.getTextColour()));
        g.fillPath(path);

        g.setColour(base_.getBackgroundColour());
        for (const auto& surface_bound : surface_bounds_) {
            g.fillRoundedRectangle(surface_bound.toFloat(), static_cast<float>(padding) * .75f);
        }
    }

    void PanelSurfaceBackground::setSurfaceBounds(std::vector<juce::Rectangle<int>> bounds) {
        surface_bounds_ = std::move(bounds);
        repaint();
    }
}
