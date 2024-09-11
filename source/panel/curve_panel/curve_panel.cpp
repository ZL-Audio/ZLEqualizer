// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "curve_panel.hpp"

namespace zlPanel {
    CurvePanel::CurvePanel(juce::AudioProcessorValueTreeState &parameters,
                           juce::AudioProcessorValueTreeState &parametersNA,
                           zlInterface::UIBase &base,
                           zlDSP::Controller<double> &c)
        : Thread("curve panel"),
          parametersRef(parameters), parametersNARef(parametersNA), uiBase(base),
          controllerRef(c),
          backgroundPanel(parameters, parametersNA, base),
          fftPanel(c.getAnalyzer(), base),
          conflictPanel(c.getConflictAnalyzer(), base),
          sumPanel(parameters, base, c, baseFilters, mainFilters),
          soloPanel(parameters, parametersNA, base, c),
          buttonPanel(parameters, parametersNA, base, c),
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
                    std::make_unique<SinglePanel>(idx, parameters, parametersNA, base, c,
                                                  baseFilters[idx], targetFilters[idx], mainFilters[idx]);
            addAndMakeVisible(*singlePanels[i]);
        }
        addAndMakeVisible(sumPanel);
        addAndMakeVisible(soloPanel);
        addAndMakeVisible(buttonPanel);
        parameterChanged(zlDSP::scale::ID, parametersRef.getRawParameterValue(zlDSP::scale::ID)->load());
        parametersRef.addParameterListener(zlDSP::scale::ID, this);
        parameterChanged(zlState::maximumDB::ID, parametersNARef.getRawParameterValue(zlState::maximumDB::ID)->load());
        parametersNARef.addParameterListener(zlState::maximumDB::ID, this);
        startThread(juce::Thread::Priority::low);
    }

    CurvePanel::~CurvePanel() {
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
        fftPanel.setBounds(bound.toNearestInt());
        conflictPanel.setBounds(bound.toNearestInt());
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            singlePanels[i]->setBounds(bound.toNearestInt());
        }
        sumPanel.setBounds(bound.toNearestInt());
        soloPanel.setBounds(bound.toNearestInt());
        buttonPanel.setBounds(bound.toNearestInt());
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

    void CurvePanel::repaintCallBack() {
        const auto &analyzer = controllerRef.getAnalyzer();
        const juce::Time nowT = juce::Time::getCurrentTime();
        if ((nowT - currentT).inMilliseconds() > uiBase.getRefreshRateMS()) {
            if ((analyzer.getPreON() || analyzer.getPostON() || analyzer.getSideON())
                && analyzer.getPathReady()) {
                fftPanel.updatePaths();
            }
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
            for (const auto &sP: singlePanels) {
                if (sP->checkRepaint()) {
                    sP->run();
                }
            }
            sumPanel.run();
        }
    }
}
