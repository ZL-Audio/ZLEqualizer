// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "main_panel.hpp"

namespace zlPanel {
    MainPanel::MainPanel(PluginProcessor &p)
        : processorRef(p), state(p.state), uiBase(p.state),
          controlPanel(p, uiBase),
          curvePanel(p, uiBase),
          statePanel(p, uiBase, uiSettingPanel),
          uiSettingPanel(p, uiBase) {
        addAndMakeVisible(curvePanel);
        addAndMakeVisible(controlPanel);
        addAndMakeVisible(statePanel);
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
            bound.setHeight(0.47f * bound.getWidth());
        }
        const auto fontSize = bound.getWidth() * 0.014287762237762238f;
        uiBase.setFontSize(fontSize);

        auto stateBound = bound.removeFromTop(fontSize * 2.625381664859529f);
        statePanel.setBounds(stateBound.toNearestInt());
        stateBound = stateBound.removeFromRight(stateBound.getHeight());
        stateBound.removeFromBottom(fontSize * .5f);

        uiSettingPanel.setBounds(getLocalBounds());

        const auto controlBound = bound.removeFromBottom(fontSize * 7.348942487176095f);
        controlPanel.setBounds(controlBound.toNearestInt());
        curvePanel.setBounds(bound.toNearestInt());
    }

    void MainPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        juce::ignoreUnused(parameterID, newValue);
        triggerAsyncUpdate();
    }

    void MainPanel::parentHierarchyChanged() {
        if (const auto peer = getPeer()) {
            peer->setCurrentRenderingEngine(uiBase.getRenderingEngine());
        }
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
