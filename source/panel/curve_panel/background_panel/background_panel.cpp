// Copyright (C) 2024 - zsliu98
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
        setInterceptsMouseClicks(false, true);
        setOpaque(true);
        addAndMakeVisible(gridPanel);
        addAndMakeVisible(scalePanel);
    }

    BackgroundPanel::~BackgroundPanel() = default;

    void BackgroundPanel::paint(juce::Graphics &g) {
        juce::ignoreUnused(g);
        g.fillAll(uiBase.getBackgroundColor());
    }

    void BackgroundPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        const auto scaleBound = bound.removeFromRight(uiBase.getFontSize() * 4.1f);
        gridPanel.setBounds(bound.toNearestInt());
        scalePanel.setBounds(scaleBound.toNearestInt());
    }
}
