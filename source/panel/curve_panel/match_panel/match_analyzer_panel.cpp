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
    MatchAnalyzerPanel::MatchAnalyzerPanel(zlFFT::AverageFFTAnalyzer<double, 251> &analyzer,
                                           zlInterface::UIBase &base)
        : analyzerRef(analyzer), uiBase(base) {
        setInterceptsMouseClicks(false, false);
    }

    MatchAnalyzerPanel::~MatchAnalyzerPanel() {
        analyzerRef.setON(false);
    }

    void MatchAnalyzerPanel::visibilityChanged() {
        analyzerRef.setON(isVisible());
    }

    void MatchAnalyzerPanel::paint(juce::Graphics &g) {
        juce::GenericScopedTryLock lock{pathLock};
        if (!lock.isLocked()) { return; }
        g.setColour(uiBase.getColourByIdx(zlInterface::preColour).withAlpha(1.f));
        g.fillPath(recentPath1);
        g.setColour(uiBase.getColourByIdx(zlInterface::sideColour).withAlpha(1.f));
        g.fillPath(recentPath2);
    }

    void MatchAnalyzerPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        leftCorner.store({bound.getX() * 0.9f, bound.getBottom() * 1.1f});
        rightCorner.store({bound.getRight() * 1.1f, bound.getBottom() * 1.1f});
        atomicBound.store(bound);
    }

    void MatchAnalyzerPanel::updatePaths() {
        analyzerRef.createPath({path1, path2}, atomicBound.load());
        for (auto &path: {&path1, &path2}) {
            if (!path->isEmpty()) {
                path->lineTo(rightCorner.load());
                path->lineTo(leftCorner.load());
                path->closeSubPath();
            }
        } {
            juce::GenericScopedLock lock{pathLock};
            recentPath1 = path1;
            recentPath2 = path2;
        }
    }
} // zlPanel
