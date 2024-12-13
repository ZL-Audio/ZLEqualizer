// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "control_panel.hpp"

#include "../../state/state_definitions.hpp"

namespace zlPanel {
    ControlPanel::ControlPanel(PluginProcessor &p,
                               zlInterface::UIBase &base)
        : parametersNARef(p.parametersNA), uiBase(base),
          leftControlPanel(p, base),
          rightControlPanel(p, base),
          matchControlPanel(p, base) {
        parameterChanged(zlState::selectedBandIdx::ID,
                         parametersNARef.getRawParameterValue(zlState::selectedBandIdx::ID)->load());
        addAndMakeVisible(leftControlPanel);
        addAndMakeVisible(rightControlPanel);
        addChildComponent(matchControlPanel);
        parametersNARef.addParameterListener(zlState::selectedBandIdx::ID, this);
    }

    ControlPanel::~ControlPanel() {
        parametersNARef.removeParameterListener(zlState::selectedBandIdx::ID, this);
    }

    void ControlPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        const auto actualBound = uiBase.getRoundedShadowRectangleArea(bound, uiBase.getFontSize(), {});
        const auto leftWidth = actualBound.getWidth() * (33.f / 63.f) + (bound.getWidth() - actualBound.getWidth()) *
                               .5f;
        auto rightBound = getLocalBounds().toFloat();
        const auto leftBound = rightBound.removeFromLeft(leftWidth);
        leftControlPanel.setBounds(leftBound.toNearestInt());
        rightControlPanel.setBounds(rightBound.toNearestInt());
        matchControlPanel.setBounds(getLocalBounds());
    }

    void ControlPanel::parameterChanged(const juce::String &parameterID, const float newValue) {
        if (parameterID == zlState::selectedBandIdx::ID) {
            bandIdx.store(static_cast<size_t>(newValue));
            triggerAsyncUpdate();
        }
    }

    void ControlPanel::handleAsyncUpdate() {
        leftControlPanel.attachGroup(bandIdx.load());
        rightControlPanel.attachGroup(bandIdx.load());
    }
} // zlPanel
