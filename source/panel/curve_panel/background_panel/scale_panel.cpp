// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "scale_panel.hpp"

#include "../../panel_definitons.hpp"

namespace zlPanel {
    ScalePanel::ScalePanel(juce::AudioProcessorValueTreeState &parametersNA, zlInterface::UIBase &base)
        : parametersNARef(parametersNA), uiBase(base),
          scaleBox("", zlState::maximumDB::choices, base) {
        attach({&scaleBox.getBox()}, {zlState::maximumDB::ID}, parametersNARef, boxAttachments);
        addAndMakeVisible(scaleBox);
        setBufferedToImage(true);
        parametersNARef.addParameterListener(zlState::maximumDB::ID, this);
    }

    ScalePanel::~ScalePanel() {
        parametersNARef.removeParameterListener(zlState::maximumDB::ID, this);
    }

    void ScalePanel::paint(juce::Graphics &g) {
        juce::ignoreUnused(g);
    }

    void ScalePanel::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
        auto boxBound = juce::Rectangle<float>(uiBase.getFontSize() * 2.f, uiBase.getFontSize() * 1.1f);
        boxBound = boxBound.withCentre({bound.getCentreX(), bound.getY()});
        scaleBox.setBounds(boxBound.toNearestInt());
    }

    void ScalePanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlState::maximumDB::ID) {
            maximumDB.store(newValue);
            triggerAsyncUpdate();
        }
    }

    void ScalePanel::handleAsyncUpdate() {
        repaint();
    }
} // zlPanel
