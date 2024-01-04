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
    MainPanel::MainPanel(juce::AudioProcessorValueTreeState &parameters,
                         juce::AudioProcessorValueTreeState &parametersNA) : controlPanel(
        parameters, parametersNA, uiBase) {
        addAndMakeVisible(controlPanel);
    }

    void MainPanel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getBackgroundColor());
        const auto bound = getLocalBounds().toFloat();
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
    }


    void MainPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {});
        const auto fontSize = bound.getHeight() * 0.0514f * 0.45f * 5;
        uiBase.setFontSize(fontSize);
        controlPanel.setBounds(bound.toNearestInt());
    }
}
