// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_analyzer_panel.hpp"

namespace zlPanel {
    MatchAnalyzerPanel::MatchAnalyzerPanel(zlEqMatch::EqMatchAnalyzer<double> &analyzer,
                                           juce::AudioProcessorValueTreeState &parametersNA,
                                           zlInterface::UIBase &base)
        : analyzerRef(analyzer), parametersNARef(parametersNA), uiBase(base) {
        setInterceptsMouseClicks(false, false);
    }

    MatchAnalyzerPanel::~MatchAnalyzerPanel() = default;

    void MatchAnalyzerPanel::paint(juce::Graphics &g) {
        juce::GenericScopedTryLock lock{pathLock};
        g.fillAll(uiBase.getColourByIdx(zlInterface::backgroundColour).withAlpha(.5f));
        const auto thickness = uiBase.getFontSize() * 0.2f * uiBase.getSumCurveThickness();
        if (!lock.isLocked()) { return; }
        g.setColour(uiBase.getColourByIdx(zlInterface::sideColour).withAlpha(.5f));
        g.strokePath(recentPath2,
                     juce::PathStrokeType(thickness,
                                          juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(uiBase.getColourByIdx(zlInterface::preColour).withAlpha(.5f));
        g.strokePath(recentPath1,
                     juce::PathStrokeType(thickness,
                                          juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(uiBase.getColorMap2(2));
        g.strokePath(recentPath3,
                     juce::PathStrokeType(thickness * 1.5f,
                                          juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    void MatchAnalyzerPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        leftCorner.store({bound.getX() * 0.9f, bound.getBottom() * 1.1f});
        rightCorner.store({bound.getRight() * 1.1f, bound.getBottom() * 1.1f});
        atomicBound.store(bound);
    }

    void MatchAnalyzerPanel::updatePaths() {
        const auto currentMaximumDB =
                zlState::maximumDB::dBs[static_cast<size_t>(
                    parametersNARef.getRawParameterValue(zlState::maximumDB::ID)->load())];
        analyzerRef.updatePaths(path1, path2, path3,
                                atomicBound.load(),
                                {-72.f, -72.f, -currentMaximumDB * 2.f}); {
            juce::GenericScopedLock lock{pathLock};
            recentPath1 = path1;
            recentPath2 = path2;
            recentPath3 = path3;
        }
        analyzerRef.checkRun();
    }
} // zlPanel
