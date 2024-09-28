// Copyright (C) 2024 - zsliu98
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
        : processorRef(p), state(p.state), uiBase(p.state),
          controlPanel(p, uiBase),
          curvePanel(p, uiBase, p.getController()),
          statePanel(p, uiBase),
          uiSettingPanel(p, uiBase), uiSettingButton(uiSettingPanel, uiBase) {
        addAndMakeVisible(curvePanel);
        addAndMakeVisible(controlPanel);
        addAndMakeVisible(statePanel);
        addAndMakeVisible(uiSettingButton);
        addChildComponent(uiSettingPanel);

        updateFFTs();

        state.addParameterListener(zlState::fftExtraTilt::ID, this);
        state.addParameterListener(zlState::fftExtraSpeed::ID, this);
        state.addParameterListener(zlState::refreshRate::ID, this);
    }

    MainPanel::~MainPanel() {
        state.removeParameterListener(zlState::fftExtraTilt::ID, this);
        state.removeParameterListener(zlState::fftExtraSpeed::ID, this);
        state.removeParameterListener(zlState::refreshRate::ID, this);
    }

    void MainPanel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getBackgroundColor());
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        if (bound.getHeight() < 0.47f * bound.getWidth()) {
            return;
        }
        const auto fontSize = bound.getWidth() * 0.014287762237762238f;
        uiBase.setFontSize(fontSize);

        auto stateBound = bound.removeFromTop(fontSize * 2.625381664859529f);
        statePanel.setBounds(stateBound.toNearestInt());
        stateBound = stateBound.removeFromRight(stateBound.getHeight());
        stateBound.removeFromBottom(fontSize * .5f);
        uiSettingButton.setBounds(stateBound.toNearestInt());
        uiSettingPanel.setBounds(getLocalBounds());

        const auto controlBound = bound.removeFromBottom(fontSize * 7.348942487176095f);
        controlPanel.setBounds(controlBound.toNearestInt());
        curvePanel.setBounds(bound.toNearestInt());
    }

    void MainPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        juce::ignoreUnused(parameterID, newValue);
        triggerAsyncUpdate();
    }

    void MainPanel::handleAsyncUpdate() {
        updateFFTs();
    }

    void MainPanel::updateFFTs() {
        for (auto &fft : {&processorRef.getController().getAnalyzer().getMultipleFFT()}) {
            fft->setExtraTilt(uiBase.getFFTExtraTilt());
            fft->setExtraSpeed(uiBase.getFFTExtraSpeed());
            fft->setRefreshRate(zlState::refreshRate::rates[uiBase.getRefreshRateID()]);
        }
        for (auto &fft : {&processorRef.getController().getConflictAnalyzer().getSyncFFT()}) {
            fft->setRefreshRate(zlState::refreshRate::rates[uiBase.getRefreshRateID()]);
        }
    }
}
