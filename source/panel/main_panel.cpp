// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "main_panel.hpp"

namespace zlpanel {
    MainPanel::MainPanel(PluginProcessor &p, zlgui::UIBase &base)
        : processor_ref_(p), state(p.state), ui_base_(base),
          controlPanel(p, ui_base_),
          curvePanel(p, ui_base_),
          scalePanel(p, ui_base_),
          statePanel(p, ui_base_, uiSettingPanel),
          uiSettingPanel(p, ui_base_),
          outputBox(p, ui_base_),
          analyzerBox(p.parameters_NA, ui_base_),
          dynamicBox(p.parameters, ui_base_),
          collisionBox(p.parameters_NA, ui_base_),
          generalBox(p.parameters, ui_base_),
          tooltipLAF(ui_base_), tooltipWindow(&curvePanel) {
        processor_ref_.getController().setEditorOn(true);
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

        state.addParameterListener(zlstate::fftExtraTilt::ID, this);
        state.addParameterListener(zlstate::fftExtraSpeed::ID, this);
        state.addParameterListener(zlstate::refreshRate::ID, this);

        ui_base_.closeAllBox();

        lookAndFeelChanged();
    }

    MainPanel::~MainPanel() {
        processor_ref_.getController().setEditorOn(false);
        state.removeParameterListener(zlstate::fftExtraTilt::ID, this);
        state.removeParameterListener(zlstate::fftExtraSpeed::ID, this);
        state.removeParameterListener(zlstate::refreshRate::ID, this);
    }

    void MainPanel::paint(juce::Graphics &g) {
        g.fillAll(ui_base_.getBackgroundColor());
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds();
        if (static_cast<float>(bound.getHeight()) < 0.47f * static_cast<float>(bound.getWidth())) {
            bound.setHeight(juce::roundToInt(0.47f * static_cast<float>(bound.getWidth())));
        }

        const auto fontSize = static_cast<float>(bound.getWidth()) * 0.014287762237762238f;
        ui_base_.setFontSize(fontSize);

        const auto stateBound = bound.removeFromTop(juce::roundToInt(fontSize * 2.625381664859529f));
        statePanel.setBounds(stateBound); {
            auto x = stateBound.getRight();
            const auto y = stateBound.getBottom() + 1 - juce::roundToInt(ui_base_.getFontSize() * .4f);
            const auto height = static_cast<float>(stateBound.getHeight()); {
                x -= static_cast<int>(ui_base_.getFontSize() * 2.5) * 3;
                x -= static_cast<int>(ui_base_.getFontSize() * 2.5) / 4;
            }
            const auto labelWidth = juce::roundToInt(height * 2.75f);
            const auto gapWidth = juce::roundToInt(height * .5f);

            x -= labelWidth / 2; {
                const auto m_bound = outputBox.getIdealBound();
                outputBox.setBounds(m_bound.withPosition(x - m_bound.getWidth() / 2, y));
            }
            x -= (labelWidth + gapWidth); {
                const auto m_bound = analyzerBox.getIdealBound();
                analyzerBox.setBounds(m_bound.withPosition(x - m_bound.getWidth() / 2, y));
            }
            x -= (labelWidth + gapWidth); {
                const auto m_bound = dynamicBox.getIdealBound();
                dynamicBox.setBounds(m_bound.withPosition(x - m_bound.getWidth() / 2, y));
            }
            x -= (labelWidth + gapWidth); {
                const auto m_bound = collisionBox.getIdealBound();
                collisionBox.setBounds(m_bound.withPosition(x - m_bound.getWidth() / 2, y));
            }
            x -= (labelWidth + gapWidth); {
                const auto m_bound = generalBox.getIdealBound();
                generalBox.setBounds(m_bound.withPosition(x - m_bound.getWidth() / 2, y));
            }
        }

        uiSettingPanel.setBounds(getLocalBounds());

        const auto controlBound = bound.removeFromBottom(juce::roundToInt(fontSize * 7.348942487176095f));
        controlPanel.setBounds(controlBound);

        const auto scaleBound = bound.removeFromRight(juce::roundToInt(ui_base_.getFontSize() * 4.2f));
        curvePanel.setBounds(bound.toNearestInt());
        scalePanel.setBounds(scaleBound.toNearestInt());
    }

    void MainPanel::parameterChanged(const juce::String &parameter_id, float new_value) {
        juce::ignoreUnused(parameter_id, new_value);
        triggerAsyncUpdate();
    }

    void MainPanel::lookAndFeelChanged() {
        tooltipWindow.setON(ui_base_.getTooltipON());
    }

    void MainPanel::parentHierarchyChanged() {
        if (const auto peer = getPeer()) {
            auto renderEngineIdx = ui_base_.getRenderingEngine();
            auto rendererList = peer->getAvailableRenderingEngines();
            rendererList.insert(0, "Auto");
            uiSettingPanel.setRendererList(rendererList);
            if (renderEngineIdx <= 0) return;
            if (renderEngineIdx >= rendererList.size()) {
                renderEngineIdx = rendererList.size();
                ui_base_.setRenderingEngine(renderEngineIdx);
                ui_base_.saveToAPVTS();
            }
            peer->setCurrentRenderingEngine(renderEngineIdx - 1);
            ui_base_.setIsRenderingHardware(!rendererList[renderEngineIdx - 1].contains("Software"));
        }
    }

    void MainPanel::handleAsyncUpdate() {
        updateFFTs();
    }

    void MainPanel::updateFFTs() {
        for (auto &fft: {&processor_ref_.getController().getAnalyzer().getMultipleFFT()}) {
            fft->setExtraTilt(ui_base_.getFFTExtraTilt());
            fft->setExtraSpeed(ui_base_.getFFTExtraSpeed());
            fft->setRefreshRate(zlstate::refreshRate::rates[ui_base_.getRefreshRateID()]);
        }
        for (auto &fft: {&processor_ref_.getController().getConflictAnalyzer().getSyncFFT()}) {
            fft->setRefreshRate(zlstate::refreshRate::rates[ui_base_.getRefreshRateID()]);
        }
    }
}
