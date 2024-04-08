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
    MainPanel::MainPanel(PluginProcessor &p)
        : state(p.state),
          controlPanel(p.parameters, p.parametersNA, uiBase),
          curvePanel(p.parameters, p.parametersNA, uiBase, p.getController()),
          statePanel(p.parameters, p.parametersNA, p.state, uiBase),
          uiSettingPanel(uiBase), uiSettingButton(uiSettingPanel, uiBase) {
        uiBase.setStyle(static_cast<size_t>(state.getRawParameterValue(zlState::uiStyle::ID)->load()));
        addAndMakeVisible(curvePanel);
        addAndMakeVisible(controlPanel);
        addAndMakeVisible(statePanel);
        addChildComponent(uiSettingButton);
        addChildComponent(uiSettingPanel);
        state.addParameterListener(zlState::uiStyle::ID, this);
    }

    MainPanel::~MainPanel() {
        state.removeParameterListener(zlState::uiStyle::ID, this);
    }

    void MainPanel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getBackgroundColor());
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        uiBase.setFontSize(bound.getWidth() * 0.014287762237762238f);

        auto stateBound = bound.removeFromTop(bound.getHeight() * .06f);
        statePanel.setBounds(stateBound.toNearestInt());
        stateBound = stateBound.removeFromRight(stateBound.getHeight());
        stateBound.removeFromBottom(uiBase.getFontSize() * .5f);
        uiSettingButton.setBounds(stateBound.toNearestInt());
        uiSettingPanel.setBounds(getLocalBounds());

        const auto controlBound = bound.removeFromBottom(bound.getWidth() * 0.105f);
        controlPanel.setBounds(controlBound.toNearestInt());

        curvePanel.setBounds(bound.toNearestInt());
    }

    void MainPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        juce::ignoreUnused(parameterID, newValue);
        triggerAsyncUpdate();
    }

    void MainPanel::handleAsyncUpdate() {
        uiSettingButton.setVisible(uiBase.getStyle() == 2);
        sendLookAndFeelChange();
    }
}
