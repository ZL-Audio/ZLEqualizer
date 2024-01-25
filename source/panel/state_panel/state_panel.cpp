// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "state_panel.hpp"

namespace zlPanel {
    StatePanel::StatePanel(juce::AudioProcessorValueTreeState &parameters,
                           juce::AudioProcessorValueTreeState &parametersNA,
                           juce::AudioProcessorValueTreeState &state,
                           zlInterface::UIBase &base)
        : parametersRef(parameters), parametersNARef(parametersNA), stateRef(state),
          uiBase(base), logoPanel(state, base) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        juce::ignoreUnused(stateRef, uiBase);
        addAndMakeVisible(logoPanel);
    }

    void StatePanel::resized() {
        auto bound = getLocalBounds().toFloat();
        const auto logoBound = bound.removeFromLeft(bound.getWidth() * .25f);
        logoPanel.setBounds(logoBound.toNearestInt());
    }

} // zlPanel
