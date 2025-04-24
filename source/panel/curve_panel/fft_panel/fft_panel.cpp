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
    FFTPanel::FFTPanel(zlFFTAnalyzer::PrePostFFTAnalyzer<double> &analyzer,
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
        if (analyzerRef.getPreON() && !recentPrePath.isEmpty()) {
            g.setColour(uiBase.getColourByIdx(zlInterface::preColour));
            g.fillPath(recentPrePath);
        }
        if (analyzerRef.getPostON() && !recentPostPath.isEmpty()) {
            g.setColour(uiBase.getTextColor().withAlpha(0.5f));
            if (uiBase.getIsRenderingHardware()) {
                g.strokePath(recentPostPath, juce::PathStrokeType{
                                 curveThickness.load(),
                                 juce::PathStrokeType::curved,
                                 juce::PathStrokeType::rounded
                             });
            } else {
                g.fillPath(recentPostStrokePath);
            }

            g.setColour(uiBase.getColourByIdx(zlInterface::postColour));
            g.fillPath(recentPostPath);
        }

        if (analyzerRef.getSideON() && !recentSidePath.isEmpty()) {
            g.setColour(uiBase.getColourByIdx(zlInterface::sideColour));
            g.fillPath(recentSidePath);
        }
    }

    void FFTPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        leftCorner.store({bound.getX() * 0.9f, bound.getBottom() * 1.1f});
        rightCorner.store({bound.getRight() * 1.1f, bound.getBottom() * 1.1f});
        atomicBound.store(bound);
        curveThickness.store(uiBase.getFontSize() * 0.1f);
    }

    void FFTPanel::updatePaths(const float physicalPixelScaleFactor) {
        analyzerRef.updatePaths(prePath, postPath, sidePath, atomicBound.load(), minimumFFTDB.load());
        for (auto &path: {&prePath, &postPath, &sidePath}) {
            if (!path->isEmpty()) {
                path->lineTo(rightCorner.load());
                path->lineTo(leftCorner.load());
                path->closeSubPath();
            }
        } {
            if (uiBase.getIsRenderingHardware()) {
                juce::GenericScopedLock lock{pathLock};
                recentPrePath = prePath;
                recentPostPath = postPath;
            } else {
                juce::PathStrokeType stroke{
                    curveThickness.load(), juce::PathStrokeType::curved, juce::PathStrokeType::rounded
                };
                stroke.createStrokedPath(postStrokePath, postPath, {}, physicalPixelScaleFactor);
                juce::GenericScopedLock lock{pathLock};
                recentPrePath = prePath;
                recentPostPath = postPath;
                recentPostStrokePath = postStrokePath;
            }
        }
        if (analyzerRef.getSideON()) {
            juce::GenericScopedLock lock{pathLock};
            recentSidePath = sidePath;
        }
    }

    void FFTPanel::visibilityChanged() {
        analyzerRef.setON(isVisible());
    }
} // zlPanel
