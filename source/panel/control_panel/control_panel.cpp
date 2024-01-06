// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "control_panel.hpp"

#include "../../state/state_definitions.hpp"

namespace zlPanel {
    ControlPanel::ControlPanel(juce::AudioProcessorValueTreeState &parameters,
                               juce::AudioProcessorValueTreeState &parametersNA,
                               zlInterface::UIBase &base)
        : parametersNARef(parametersNA),
          leftControlPanel(parameters, parametersNA, base),
          rightControlPanel(parameters, parametersNA, base) {
        parameterChanged(zlState::selectedBandIdx::ID, static_cast<float>(zlState::selectedBandIdx::defaultI));
        addAndMakeVisible(leftControlPanel);
        addAndMakeVisible(rightControlPanel);
        parametersNARef.addParameterListener(zlState::selectedBandIdx::ID, this);
    }

    ControlPanel::~ControlPanel() {
        parametersNARef.removeParameterListener(zlState::selectedBandIdx::ID, this);
    }


    void ControlPanel::resized() {
        auto rightBound = getLocalBounds().toFloat();
        const auto leftBound = rightBound.removeFromLeft(rightBound.getWidth() * (33.f / 61.f));
        leftControlPanel.setBounds(leftBound.toNearestInt());
        rightControlPanel.setBounds(rightBound.toNearestInt());
    }

    void ControlPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlState::selectedBandIdx::ID) {
            const auto bandIdx = static_cast<size_t> (newValue);
            leftControlPanel.attachGroup(bandIdx);
            rightControlPanel.attachGroup(bandIdx);
        }
    }
} // zlPanel
