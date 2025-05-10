// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fft_panel.hpp"

namespace zlpanel {
    FFTPanel::FFTPanel(zldsp::analyzer::PrePostFFTAnalyzer<double> &analyzer,
                       zlgui::UIBase &base)
        : analyzerRef(analyzer), ui_base_(base) {
        setInterceptsMouseClicks(false, false);
    }

    FFTPanel::~FFTPanel() {
        analyzerRef.setON(false);
    }

    void FFTPanel::paint(juce::Graphics &g) {
        juce::GenericScopedTryLock lock{pathLock};
        if (!lock.isLocked()) { return; }
        if (analyzerRef.getPreON() && !recentPrePath.isEmpty()) {
            g.setColour(ui_base_.getColourByIdx(zlgui::kPreColour));
            g.fillPath(recentPrePath);
        }
        if (analyzerRef.getPostON() && !recentPostPath.isEmpty()) {
            g.setColour(ui_base_.getTextColor().withAlpha(0.5f));
            if (ui_base_.getIsRenderingHardware()) {
                g.strokePath(recentPostPath, juce::PathStrokeType{
                                 curveThickness.load(),
                                 juce::PathStrokeType::curved,
                                 juce::PathStrokeType::rounded
                             });
            } else {
                g.fillPath(recentPostStrokePath);
            }

            g.setColour(ui_base_.getColourByIdx(zlgui::kPostColour));
            g.fillPath(recentPostPath);
        }

        if (analyzerRef.getSideON() && !recentSidePath.isEmpty()) {
            g.setColour(ui_base_.getColourByIdx(zlgui::kSideColour));
            g.fillPath(recentSidePath);
        }
    }

    void FFTPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        leftCorner.store({bound.getX() * 0.9f, bound.getBottom() * 1.1f});
        rightCorner.store({bound.getRight() * 1.1f, bound.getBottom() * 1.1f});
        atomicBound.store(bound);
        curveThickness.store(ui_base_.getFontSize() * 0.1f);
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
            if (ui_base_.getIsRenderingHardware()) {
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
} // zlpanel
