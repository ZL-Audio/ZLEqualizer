// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fft_panel.hpp"

namespace zlPanel {
    FFTPanel::FFTPanel(zlFFT::PrePostFFTAnalyzer<double> &analyzer,
                       zlInterface::UIBase &base)
        : analyzerRef(analyzer), uiBase(base) {
        setInterceptsMouseClicks(false, false);
    }

    FFTPanel::~FFTPanel() {
        analyzerRef.setON(false);
    }

    void FFTPanel::paint(juce::Graphics &g) {
        juce::GenericScopedTryLock lock{pathLock};
        if (!lock.isLocked()) { return; }
        if (analyzerRef.getPreON() && !recentPath1.isEmpty()) {
            g.setColour(uiBase.getColourByIdx(zlInterface::preColour));
            g.fillPath(recentPath1);
        }

        if (analyzerRef.getPostON() && !recentPath2.isEmpty()) {
            g.setColour(uiBase.getTextColor().withAlpha(0.5f));
            const auto thickness = uiBase.getFontSize() * 0.1f;
            g.strokePath(recentPath2,
                         juce::PathStrokeType(thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            g.setColour(uiBase.getColourByIdx(zlInterface::postColour));
            g.fillPath(recentPath2);
        }

        if (analyzerRef.getSideON() && !recentPath3.isEmpty()) {
            g.setColour(uiBase.getColourByIdx(zlInterface::sideColour));
            g.fillPath(recentPath3);
        }
    }

    void FFTPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        leftCorner.store({bound.getX() * 0.9f, bound.getBottom() * 1.1f});
        rightCorner.store({bound.getRight() * 1.1f, bound.getBottom() * 1.1f});
        atomicBound.store(bound);
    }

    void FFTPanel::updatePaths() {
        analyzerRef.updatePaths(path1, path2, path3, atomicBound.load(), minimumFFTDB.load());
        for (auto &path: {&path1, &path2, &path3}) {
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
        if (analyzerRef.getSideON()) {
            juce::GenericScopedLock lock{pathLock};
            recentPath3 = path3;
        }
    }

    void FFTPanel::visibilityChanged() {
        analyzerRef.setON(isVisible());
    }
} // zlPanel
