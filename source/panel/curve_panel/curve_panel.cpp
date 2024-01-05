// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "curve_panel.hpp"

namespace zlPanel {
    CurvePanel::CurvePanel(juce::AudioProcessorValueTreeState &parameters,
                           juce::AudioProcessorValueTreeState &parametersNA,
                           zlInterface::UIBase &base,
                           zlDSP::Controller<float> &c)
        : uiBase(base),
          backgroundPanel(base),
          sumPanel(base, c) {
        addAndMakeVisible(backgroundPanel);
        addAndMakeVisible(sumPanel);
        juce::ignoreUnused(parameters, parametersNA);
    }

    CurvePanel::~CurvePanel() = default;

    void CurvePanel::paint(juce::Graphics &g) {
        juce::ignoreUnused(g);
    }

    void CurvePanel::resized() {
        backgroundPanel.setBounds(getLocalBounds());
        auto bound = getLocalBounds().toFloat();
        bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());
        sumPanel.setBounds(bound.toNearestInt());
    }
}
