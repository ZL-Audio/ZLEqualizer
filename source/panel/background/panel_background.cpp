// Copyright (C) 2026 - zsliu98
// This file is part of ZLSpectrumEqualizer
//
// ZLSpectrumEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSpectrumEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSpectrumEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "panel_background.hpp"

#include <utility>

#include "../helper/panel_constants.hpp"

namespace zlpanel {
    namespace {
        constexpr auto kSurfaceTint = .05f;
    }

    PanelBackground::PanelBackground(zlgui::UIBase& base, const float shadow_alpha) :
        base_(base), shadow_alpha_(shadow_alpha) {
        setInterceptsMouseClicks(false, false);
        setAlpha(.9f);
    }

    void PanelBackground::paint(juce::Graphics& g) {
        const auto padding = getPaddingSize(base_.getFontSize());
        const auto bound = getLocalBounds().reduced(padding);
        juce::Path path;
        path.addRoundedRectangle(bound.toFloat(), static_cast<float>(padding));

        const juce::DropShadow shadow{base_.getTextColour().withAlpha(shadow_alpha_), padding, {0, 0}};
        shadow.drawForPath(g, path);
        const auto background = base_.getBackgroundColour();
        g.setColour(paints_surfaces_
                        ? background.interpolatedWith(base_.getTextColour(), kSurfaceTint)
                        : background);
        g.fillPath(path);

        if (paints_surfaces_) {
            g.setColour(background);
            for (const auto& surface_bound : surface_bounds_) {
                g.fillRoundedRectangle(surface_bound.toFloat(), static_cast<float>(padding) * .75f);
            }
        }
    }

    void PanelBackground::setSurfaceBounds(std::vector<juce::Rectangle<int>> bounds) {
        surface_bounds_ = std::move(bounds);
        paints_surfaces_ = true;
        repaint();
    }
}
