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
          uiBase(base),
          logoPanel(state, base),
          fftSettingPanel(parameters, parametersNA, base),
          compSettingPanel(parameters, parametersNA, base),
          outputSettingPanel(parameters, parametersNA, base) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        juce::ignoreUnused(stateRef, uiBase);
        setInterceptsMouseClicks(false, true);
        addAndMakeVisible(logoPanel);
        addAndMakeVisible(fftSettingPanel);
        addAndMakeVisible(compSettingPanel);
        addAndMakeVisible(outputSettingPanel);
    }

    void StatePanel::resized() {
        auto bound = getLocalBounds().toFloat();
        const auto logoBound = bound.removeFromLeft(bound.getWidth() * .125f);
        logoPanel.setBounds(logoBound.toNearestInt());
        bound.removeFromRight(uiBase.getFontSize() * 4);
        const auto height = bound.getHeight();
        bound.removeFromBottom(uiBase.getFontSize() * .5f);
        const auto fftSettingBound = bound.removeFromRight(height * 2.5f);
        fftSettingPanel.setBounds(fftSettingBound.toNearestInt());
        bound.removeFromRight(height * .5f);
        const auto compSettingBound = bound.removeFromRight(height * 2.5f);
        compSettingPanel.setBounds(compSettingBound.toNearestInt());
        bound.removeFromRight(height * .5f);
        const auto outputSettingBound = bound.removeFromRight(height * 2.5f);
        outputSettingPanel.setBounds(outputSettingBound.toNearestInt());
    }
} // zlPanel
