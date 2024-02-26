// Copyright (C) 2023 - zsliu98
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
        : parametersNARef(parametersNA), uiBase(base),
          controllerRef(c),
          backgroundPanel(parameters, parametersNA, base),
          fftPanel(c.getAnalyzer(), base),
          sumPanel(parameters, base, c),
          soloPanel(parameters, parametersNA, base, c),
          buttonPanel(parameters, parametersNA, base),
          currentT(juce::Time::getCurrentTime()),
          vblank(this, [this]() { repaintCallBack(); }) {
        addAndMakeVisible(backgroundPanel);
        addAndMakeVisible(fftPanel);
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            singlePanels[i] = std::make_unique<
                SinglePanel>(zlState::bandNUM - i - 1, parameters, parametersNA, base, c);
            addAndMakeVisible(*singlePanels[i]);
        }
        addAndMakeVisible(sumPanel);
        addAndMakeVisible(soloPanel);
        addAndMakeVisible(buttonPanel);
        parameterChanged(zlState::maximumDB::ID, parametersNA.getRawParameterValue(zlState::maximumDB::ID)->load());
        parametersNARef.addParameterListener(zlState::maximumDB::ID, this);
    }

    CurvePanel::~CurvePanel() {
        // stopTimer();
        parametersNARef.removeParameterListener(zlState::maximumDB::ID, this);
    }

    void CurvePanel::paint(juce::Graphics &g) {
        juce::ignoreUnused(g);
    }

    void CurvePanel::resized() {
        backgroundPanel.setBounds(getLocalBounds());
        auto bound = getLocalBounds().toFloat();
        bound.removeFromRight(uiBase.getFontSize() * 4);
        fftPanel.setBounds(bound.toNearestInt());
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
        }
    }

    void CurvePanel::repaintCallBack() {
        auto &analyzer = controllerRef.getAnalyzer();
        const juce::Time nowT = juce::Time::getCurrentTime();
        if ((analyzer.getPreON() || analyzer.getPostON() || analyzer.getSideON())
            && analyzer.isFFTReady()) {
            fftPanel.repaint();
            currentT = nowT;
        } else if ((nowT - currentT).inMilliseconds() > 16) {
            sumPanel.repaint();
            currentT = nowT;
        }
    }
}
