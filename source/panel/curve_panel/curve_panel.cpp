// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "curve_panel.hpp"

namespace zlPanel {
    CurvePanel::CurvePanel(PluginProcessor &processor,
                           zlInterface::UIBase &base)
        : Thread("curve panel"),
          processorRef(processor),
          parametersRef(processor.parameters), parametersNARef(processor.parametersNA), uiBase(base),
          controllerRef(processor.getController()),
          backgroundPanel(parametersRef, parametersNARef, base),
          fftPanel(controllerRef.getAnalyzer(), base),
          conflictPanel(controllerRef.getConflictAnalyzer(), base),
          sumPanel(parametersRef, base, controllerRef, baseFilters, mainFilters),
          loudnessDisplay(processorRef, base),
          buttonPanel(processorRef, base),
          soloPanel(parametersRef, parametersNARef, base, controllerRef, buttonPanel),
          matchPanel(processor.getController().getMatchAnalyzer(), parametersNARef, base) {
        for (auto &filters: {&baseFilters, &targetFilters, &mainFilters}) {
            for (auto &f: *filters) {
                f.prepare(48000.0);
                f.prepareDBSize(ws.size());
            }
        }
        addAndMakeVisible(backgroundPanel, 0);
        addAndMakeVisible(fftPanel, 1);
        addChildComponent(conflictPanel, 2);
        parametersNARef.addParameterListener(zlState::selectedBandIdx::ID, this);
        parameterChanged(zlState::selectedBandIdx::ID,
                         parametersNARef.getRawParameterValue(zlState::selectedBandIdx::ID)->load());
        addAndMakeVisible(dummyComponent, 4);
        dummyComponent.setInterceptsMouseClicks(false, false);
        for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
            const auto i = idx;
            singlePanels[idx] =
                    std::make_unique<SinglePanel>(i, parametersRef, parametersNARef, base, controllerRef,
                                                  baseFilters[i], targetFilters[i], mainFilters[i]);
            dummyComponent.addAndMakeVisible(*singlePanels[idx]);
        }
        for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
            const auto i = idx;
            sidePanels[idx] = std::make_unique<SidePanel>(i, parametersRef, parametersNARef, base, controllerRef,
                                                          buttonPanel.getSideDragger(i));
            addAndMakeVisible(*sidePanels[idx], 5);
        }
        addAndMakeVisible(sumPanel, 6);
        addChildComponent(soloPanel, 7);
        addAndMakeVisible(loudnessDisplay, 8);
        addAndMakeVisible(buttonPanel, 9);
        addChildComponent(matchPanel, 10);
        parameterChanged(zlDSP::scale::ID, parametersRef.getRawParameterValue(zlDSP::scale::ID)->load());
        parametersRef.addParameterListener(zlDSP::scale::ID, this);
        parameterChanged(zlState::maximumDB::ID, parametersNARef.getRawParameterValue(zlState::maximumDB::ID)->load());
        parametersNARef.addParameterListener(zlState::maximumDB::ID, this);
        parameterChanged(zlState::minimumFFTDB::ID,
                         parametersNARef.getRawParameterValue(zlState::minimumFFTDB::ID)->load());
        parametersNARef.addParameterListener(zlState::minimumFFTDB::ID, this);
        startThread(juce::Thread::Priority::low);

        uiBase.getValueTree().addListener(this);
    }

    CurvePanel::~CurvePanel() {
        uiBase.getValueTree().removeListener(this);
        if (isThreadRunning()) {
            stopThread(-1);
        }
        parametersRef.removeParameterListener(zlDSP::scale::ID, this);
        parametersNARef.removeParameterListener(zlState::selectedBandIdx::ID, this);
        parametersNARef.removeParameterListener(zlState::maximumDB::ID, this);
        parametersNARef.removeParameterListener(zlState::minimumFFTDB::ID, this);
    }

    void CurvePanel::paint(juce::Graphics &g) {
        juce::ignoreUnused(g);
        if (!uiBase.getIsRenderingHardware()) {
            physicalPixelScaleFactor.store(g.getInternalContext().getPhysicalPixelScaleFactor());
        }
    }

    void CurvePanel::paintOverChildren(juce::Graphics &g) {
        juce::ignoreUnused(g);
        if (toNotify) {
            toNotify = false;
            notify();
        }
    }

    void CurvePanel::resized() {
        const auto bound = getLocalBounds();
        backgroundPanel.setBounds(bound);
        fftPanel.setBounds(bound);
        conflictPanel.setBounds(bound);
        dummyComponent.setBounds(bound);
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            singlePanels[i]->setBounds(bound);
        }
        const auto sideBound = bound.toFloat().withTop(bound.toFloat().getBottom() - 2.f * uiBase.getFontSize());
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            sidePanels[i]->setBounds(sideBound.toNearestInt());
        }
        sumPanel.setBounds(bound);
        soloPanel.setBounds(bound);
        buttonPanel.setBounds(bound);
        matchPanel.setBounds(bound);

        auto lBound = getLocalBounds().toFloat();
        lBound = juce::Rectangle<float>(lBound.getX() + lBound.getWidth() * 0.666f,
                                        lBound.getBottom() - uiBase.getFontSize() * .5f,
                                        lBound.getWidth() * .09f, uiBase.getFontSize() * .5f);
        loudnessDisplay.setBounds(lBound.toNearestInt());
    }

    void CurvePanel::parameterChanged(const juce::String &parameterID, const float newValue) {
        if (parameterID == zlState::selectedBandIdx::ID) {
            currentBandIdx.store(static_cast<size_t>(newValue));
        } else if (parameterID == zlState::maximumDB::ID) {
            const auto idx = static_cast<size_t>(newValue);
            const auto maxDB = zlState::maximumDB::dBs[idx];
            sumPanel.setMaximumDB(maxDB);
            for (size_t i = 0; i < zlState::bandNUM; ++i) {
                singlePanels[i]->setMaximumDB(maxDB);
            }
        } else if (parameterID == zlDSP::scale::ID) {
            const auto scale = static_cast<double>(zlDSP::scale::formatV(newValue));
            for (size_t i = 0; i < zlState::bandNUM; ++i) {
                singlePanels[i]->setScale(scale);
            }
        } else if (parameterID == zlState::minimumFFTDB::ID) {
            const auto idx = static_cast<size_t>(newValue);
            const auto minDB = zlState::minimumFFTDB::dBs[idx];
            fftPanel.setMinimumFFTDB(minDB);
        }
    }

    void CurvePanel::valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &property) {
        if (property == zlInterface::identifiers[static_cast<size_t>(zlInterface::settingIdx::matchPanelShow)]) {
            const auto f = static_cast<bool>(uiBase.getProperty(zlInterface::settingIdx::matchPanelShow));
            showMatchPanel.store(f);
            matchPanel.setVisible(f);
            buttonPanel.setVisible(!f);
            loudnessDisplay.updateVisible(!f);
            if (f) { soloPanel.turnOffSolo(); }
        } else if (property == zlInterface::identifiers[static_cast<size_t>(
                       zlInterface::settingIdx::uiSettingPanelShow)]) {
            const auto f = static_cast<bool>(uiBase.getProperty(zlInterface::settingIdx::uiSettingPanelShow));
            showUISettingsPanel = f;
        }
    }

    void CurvePanel::repaintCallBack(const double nowT) {
        if (showUISettingsPanel) { return; }
        const auto refreshRateMul = showMatchPanel.load() ? 2.0 : 1.0;
        if ((nowT - currentT) * 1000.0 > static_cast<double>(uiBase.getRefreshRateMS()) * refreshRateMul) {
            buttonPanel.updateAttach();
            auto isCurrentDraggerMoved = false;
            for (size_t i = 0; i < zlState::bandNUM; ++i) {
                const auto f = buttonPanel.updateDragger(i, singlePanels[i]->getButtonPos());
                if (i == previousBandIdx) isCurrentDraggerMoved = f;
            }
            if (previousBandIdx != currentBandIdx.load()) {
                if (previousBandIdx < zlState::bandNUM) {
                    buttonPanel.updateLinkButton(previousBandIdx);
                }
                previousBandIdx = currentBandIdx.load();
                buttonPanel.updatePopup(previousBandIdx);
                buttonPanel.updateLinkButton(previousBandIdx);
            } else {
                buttonPanel.updateOtherDraggers(previousBandIdx,
                                                singlePanels[previousBandIdx]->getTargetButtonPos());
                buttonPanel.updatePopup(previousBandIdx, isCurrentDraggerMoved);
                buttonPanel.updateLinkButton(previousBandIdx);
            }

            conflictPanel.updateGradient();
            loudnessDisplay.checkVisible();
            soloPanel.checkVisible();
            for (const auto &panel: singlePanels) {
                panel->updateVisible();
            }
            singlePanels[currentBandIdx.load()]->toFront(false);
            for (const auto &panel: sidePanels) {
                panel->updateDragger();
            }
            if (showMatchPanel.load()) {
                matchPanel.updateDraggers();
            }
            if (!toNotify) {
                toNotify = true;
                repaint();
                currentT = nowT;
            }
        }
    }

    void CurvePanel::run() {
        juce::ScopedNoDenormals noDenormals;
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            const auto factor = physicalPixelScaleFactor.load();
            const auto &analyzer = controllerRef.getAnalyzer();
            if (analyzer.getPreON() || analyzer.getPostON() || analyzer.getSideON()) {
                fftPanel.updatePaths(factor);
            }
            for (size_t i = 0; i < zlState::bandNUM; ++i) {
                if (singlePanels[i]->checkRepaint()) {
                    singlePanels[i]->run(factor);
                }
            }
            sumPanel.run(factor);
            if (showMatchPanel.load()) {
                matchPanel.updatePaths();
            }
        }
    }
}
