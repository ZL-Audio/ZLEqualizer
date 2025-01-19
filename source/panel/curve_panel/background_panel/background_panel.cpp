// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

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
        g.fillAll(uiBase.getBackgroundColor());
    }

    void BackgroundPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        const auto scaleBound = bound.removeFromRight(uiBase.getFontSize() * 4.2f);
        gridPanel.setBounds(bound.toNearestInt());
        scalePanel.setBounds(scaleBound.toNearestInt());
    }
}
