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
    MainPanel::MainPanel(PluginProcessor &p, zlInterface::UIBase &base)
        : processorRef(p), state(p.state), uiBase(base),
          controlPanel(p, uiBase),
          curvePanel(p, uiBase),
          scalePanel(p, uiBase),
          statePanel(p, uiBase, uiSettingPanel),
          uiSettingPanel(p, uiBase),
          outputBox(p, uiBase),
          analyzerBox(p.parametersNA, uiBase),
          dynamicBox(p.parameters, uiBase),
          collisionBox(p.parametersNA, uiBase),
          generalBox(p.parameters, uiBase),
          tooltipLAF(uiBase), tooltipWindow(&curvePanel) {
        processorRef.getController().setEditorOn(true);
        addAndMakeVisible(curvePanel);
        addAndMakeVisible(scalePanel);
        addAndMakeVisible(controlPanel);
        addAndMakeVisible(statePanel);
        addChildComponent(uiSettingPanel);

        addChildComponent(outputBox);
        addChildComponent(analyzerBox);
        addChildComponent(dynamicBox);
        addChildComponent(collisionBox);
        addChildComponent(generalBox);

        tooltipWindow.setLookAndFeel(&tooltipLAF);
        tooltipWindow.setOpaque(false);
        tooltipWindow.setBufferedToImage(true);

        updateFFTs();

        state.addParameterListener(zlState::fftExtraTilt::ID, this);
        state.addParameterListener(zlState::fftExtraSpeed::ID, this);
        state.addParameterListener(zlState::refreshRate::ID, this);

        uiBase.closeAllBox();

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
        auto bound = getLocalBounds();
        if (static_cast<float>(bound.getHeight()) < 0.47f * static_cast<float>(bound.getWidth())) {
            bound.setHeight(juce::roundToInt(0.47f * static_cast<float>(bound.getWidth())));
        }

        const auto fontSize = static_cast<float>(bound.getWidth()) * 0.014287762237762238f;
        uiBase.setFontSize(fontSize);

        const auto stateBound = bound.removeFromTop(juce::roundToInt(fontSize * 2.625381664859529f));
        statePanel.setBounds(stateBound); {
            auto x = stateBound.getRight();
            const auto y = stateBound.getBottom() + 1 - juce::roundToInt(uiBase.getFontSize() * .4f);
            const auto height = static_cast<float>(stateBound.getHeight()); {
                x -= static_cast<int>(uiBase.getFontSize() * 2.5) * 3;
                x -= static_cast<int>(uiBase.getFontSize() * 2.5) / 4;
            }
            const auto labelWidth = juce::roundToInt(height * 2.75f);
            const auto gapWidth = juce::roundToInt(height * .5f);

            x -= labelWidth / 2; {
                const auto mBound = outputBox.getIdealBound();
                outputBox.setBounds(mBound.withPosition(x - mBound.getWidth() / 2, y));
            }
            x -= (labelWidth + gapWidth); {
                const auto mBound = analyzerBox.getIdealBound();
                analyzerBox.setBounds(mBound.withPosition(x - mBound.getWidth() / 2, y));
            }
            x -= (labelWidth + gapWidth); {
                const auto mBound = dynamicBox.getIdealBound();
                dynamicBox.setBounds(mBound.withPosition(x - mBound.getWidth() / 2, y));
            }
            x -= (labelWidth + gapWidth); {
                const auto mBound = collisionBox.getIdealBound();
                collisionBox.setBounds(mBound.withPosition(x - mBound.getWidth() / 2, y));
            }
            x -= (labelWidth + gapWidth); {
                const auto mBound = generalBox.getIdealBound();
                generalBox.setBounds(mBound.withPosition(x - mBound.getWidth() / 2, y));
            }
        }

        uiSettingPanel.setBounds(getLocalBounds());

        const auto controlBound = bound.removeFromBottom(juce::roundToInt(fontSize * 7.348942487176095f));
        controlPanel.setBounds(controlBound);

        const auto scaleBound = bound.removeFromRight(juce::roundToInt(uiBase.getFontSize() * 4.2f));
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
            auto rendererList = peer->getAvailableRenderingEngines();
            rendererList.insert(0, "Auto");
            uiSettingPanel.setRendererList(rendererList);
            if (renderEngineIdx <= 0) return;
            if (renderEngineIdx >= rendererList.size()) {
                renderEngineIdx = rendererList.size();
                uiBase.setRenderingEngine(renderEngineIdx);
                uiBase.saveToAPVTS();
            }
            peer->setCurrentRenderingEngine(renderEngineIdx - 1);
            uiBase.setIsRenderingHardware(!rendererList[renderEngineIdx - 1].contains("Software"));
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
