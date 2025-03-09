// Copyright (C) 2025 - zsliu98
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
          scalePanel(p, uiBase),
          statePanel(p, uiBase, uiSettingPanel),
          uiSettingPanel(p, uiBase),
          tooltipLAF(uiBase), tooltipWindow(&curvePanel) {
        processorRef.getController().setEditorOn(true);
        addAndMakeVisible(curvePanel);
        addAndMakeVisible(scalePanel);
        addAndMakeVisible(controlPanel);
        addAndMakeVisible(statePanel);
        addChildComponent(uiSettingPanel);

        tooltipWindow.setLookAndFeel(&tooltipLAF);
        tooltipWindow.setOpaque(false);
        tooltipWindow.setBufferedToImage(true);

        updateFFTs();

        state.addParameterListener(zlState::fftExtraTilt::ID, this);
        state.addParameterListener(zlState::fftExtraSpeed::ID, this);
        state.addParameterListener(zlState::refreshRate::ID, this);

        lookAndFeelChanged();
    }

    MainPanel::~MainPanel() {
        processorRef.getController().setEditorOn(false);
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
        const auto scaleBound = bound.removeFromRight(uiBase.getFontSize() * 4.2f);
        curvePanel.setBounds(bound.toNearestInt());
        scalePanel.setBounds(scaleBound.toNearestInt());
    }

    void MainPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        juce::ignoreUnused(parameterID, newValue);
        triggerAsyncUpdate();
    }

    void MainPanel::lookAndFeelChanged() {
        tooltipWindow.setON(uiBase.getTooltipON());
    }

    void MainPanel::parentHierarchyChanged() {
        if (const auto peer = getPeer()) {
            auto renderEngineIdx = uiBase.getRenderingEngine();
            const auto rendererList = peer->getAvailableRenderingEngines();
            uiSettingPanel.setRendererList(rendererList);
            if (renderEngineIdx >= rendererList.size()) {
                renderEngineIdx = rendererList.size() - 1;
                uiBase.setRenderingEngine(renderEngineIdx);
                uiBase.saveToAPVTS();
            }
            peer->setCurrentRenderingEngine(renderEngineIdx);
            uiBase.setIsRenderingHardware(!rendererList[renderEngineIdx].contains("Software"));
        }
    }

    void MainPanel::handleAsyncUpdate() {
        updateFFTs();
    }

    void MainPanel::updateFFTs() {
        for (auto &fft: {&processorRef.getController().getAnalyzer().getMultipleFFT()}) {
            fft->setExtraTilt(uiBase.getFFTExtraTilt());
            fft->setExtraSpeed(uiBase.getFFTExtraSpeed());
            fft->setRefreshRate(zlState::refreshRate::rates[uiBase.getRefreshRateID()]);
        }
        for (auto &fft: {&processorRef.getController().getConflictAnalyzer().getSyncFFT()}) {
            fft->setRefreshRate(zlState::refreshRate::rates[uiBase.getRefreshRateID()]);
        }
    }
}
