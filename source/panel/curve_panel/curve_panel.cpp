// Copyright (C) 2024 - zsliu98
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
          soloPanel(parametersRef, parametersNARef, base, controllerRef),
          buttonPanel(processorRef, base),
          matchPanel(processor.getController().getMatchAnalyzer(), base),
          currentT(juce::Time::getCurrentTime()),
          vblank(this, [this]() { repaintCallBack(); }) {
        for (auto &filters: {&baseFilters, &targetFilters, &mainFilters}) {
            for (auto &f: *filters) {
                f.prepare(48000.0);
                f.prepareDBSize(ws.size());
            }
        }
        addAndMakeVisible(backgroundPanel);
        addAndMakeVisible(fftPanel);
        addAndMakeVisible(conflictPanel);
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            const auto idx = zlState::bandNUM - i - 1;
            singlePanels[i] =
                    std::make_unique<SinglePanel>(idx, parametersRef, parametersNARef, base, controllerRef,
                                                  baseFilters[idx], targetFilters[idx], mainFilters[idx]);
            addAndMakeVisible(*singlePanels[i]);
        }
        addAndMakeVisible(sumPanel);
        addAndMakeVisible(soloPanel);
        addAndMakeVisible(buttonPanel);
        addChildComponent(matchPanel);
        parameterChanged(zlDSP::scale::ID, parametersRef.getRawParameterValue(zlDSP::scale::ID)->load());
        parametersRef.addParameterListener(zlDSP::scale::ID, this);
        parameterChanged(zlState::maximumDB::ID, parametersNARef.getRawParameterValue(zlState::maximumDB::ID)->load());
        parametersNARef.addParameterListener(zlState::maximumDB::ID, this);
        startThread(juce::Thread::Priority::low);

        uiBase.getValueTree().addListener(this);
    }

    CurvePanel::~CurvePanel() {
        uiBase.getValueTree().removeListener(this);
        if (isThreadRunning()) {
            stopThread(-1);
        }
        parametersRef.removeParameterListener(zlDSP::scale::ID, this);
        parametersNARef.removeParameterListener(zlState::maximumDB::ID, this);
    }

    void CurvePanel::paint(juce::Graphics &g) {
        juce::ignoreUnused(g);
    }

    void CurvePanel::paintOverChildren(juce::Graphics &g) {
        juce::ignoreUnused(g);
        notify();
    }

    void CurvePanel::resized() {
        backgroundPanel.setBounds(getLocalBounds());
        auto bound = getLocalBounds().toFloat();
        bound.removeFromRight(uiBase.getFontSize() * 4.1f);
        const auto intBound = bound.toNearestInt();
        fftPanel.setBounds(intBound);
        conflictPanel.setBounds(intBound);
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            singlePanels[i]->setBounds(intBound);
        }
        sumPanel.setBounds(intBound);
        soloPanel.setBounds(intBound);
        buttonPanel.setBounds(intBound);
        matchPanel.setBounds(intBound);
    }

    void CurvePanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlState::maximumDB::ID) {
            const auto idx = static_cast<size_t>(newValue);
            const auto maxDB = zlState::maximumDB::dBs[idx];
            backgroundPanel.setMaximumDB(maxDB);
            sumPanel.setMaximumDB(maxDB);
            for (size_t i = 0; i < zlState::bandNUM; ++i) {
                singlePanels[i]->setMaximumDB(maxDB);
            }
        } else if (parameterID == zlDSP::scale::ID) {
            const auto scale = static_cast<double>(zlDSP::scale::formatV(newValue));
            for (size_t i = 0; i < zlState::bandNUM; ++i) {
                singlePanels[i]->setScale(scale);
            }
        }
    }

    void CurvePanel::valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &) {
        const auto f = static_cast<bool>(uiBase.getProperty(zlInterface::settingIdx::matchPanelShow));
        showMatchPanel.store(f);
        matchPanel.setVisible(f);
        buttonPanel.setVisible(!f);
        soloPanel.setVisible(!f);
    }

    void CurvePanel::repaintCallBack() {
        const juce::Time nowT = juce::Time::getCurrentTime();
        if ((nowT - currentT).inMilliseconds() > uiBase.getRefreshRateMS()) {
            buttonPanel.updateDraggers();
            conflictPanel.updateGradient();
            repaint();
            currentT = nowT;
        }
    }

    void CurvePanel::run() {
        juce::ScopedNoDenormals noDenormals;
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            const auto &analyzer = controllerRef.getAnalyzer();
            if (analyzer.getPreON() || analyzer.getPostON() || analyzer.getSideON()) {
                fftPanel.updatePaths();
            }
            for (const auto &sP: singlePanels) {
                if (sP->checkRepaint()) {
                    sP->run();
                }
            }
            sumPanel.run();
            if (showMatchPanel.load()) {
                matchPanel.updatePaths();
            }
        }
    }
}
