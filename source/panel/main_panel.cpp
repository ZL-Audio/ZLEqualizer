// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "main_panel.hpp"

namespace zlPanel {
    MainPanel::MainPanel(PluginProcessor &p)
        : controlPanel(p.parameters, p.parametersNA, uiBase),
          curvePanel(p.parameters, p.parametersNA, uiBase, p.getController()) {
        addAndMakeVisible(curvePanel);
        addAndMakeVisible(controlPanel);
    }

    void MainPanel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getBackgroundColor());
        // const auto bound = getLocalBounds().toFloat();
        // uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        uiBase.setFontSize(bound.getWidth() * 0.014287762237762238f);
        // bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {});

        // const auto controlBound = bound.removeFromBottom(bound.getWidth() * 0.12354312354312354f);
        const auto controlBound = bound.removeFromBottom(bound.getWidth() * 0.11f);
        controlPanel.setBounds(controlBound.toNearestInt());
        curvePanel.setBounds(bound.toNearestInt());
    }
}
