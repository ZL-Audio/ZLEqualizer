// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "background_panel.hpp"

namespace zlPanel {
    BackgroundPanel::BackgroundPanel(juce::AudioProcessorValueTreeState &parameters,
                                 juce::AudioProcessorValueTreeState &parametersNA,
                                 zlInterface::UIBase &base)
        : uiBase(base),
          gridPanel(base),
    scalePanel(parametersNA, base){
        juce::ignoreUnused(parameters);
        addAndMakeVisible(gridPanel);
        addAndMakeVisible(scalePanel);
        setBufferedToImage(true);
    }

    BackgroundPanel::~BackgroundPanel() = default;

    void BackgroundPanel::paint(juce::Graphics &g) {
        auto bound = getLocalBounds().toFloat();
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
    }

    void BackgroundPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
        const auto scaleBound = bound.removeFromRight(uiBase.getFontSize() * 4);
        gridPanel.setBounds(bound.toNearestInt());
        scalePanel.setBounds(scaleBound.toNearestInt());
    }
}
