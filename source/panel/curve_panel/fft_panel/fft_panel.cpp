// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fft_panel.hpp"

namespace zlPanel {
    FFTPanel::FFTPanel(zlFFT::PrePostFFTAnalyzer<double> &analyzer,
                       zlInterface::UIBase &base)
        : analyzerRef(analyzer), uiBase(base) {
        setInterceptsMouseClicks(false, false);
        analyzerRef.setON(true);
    }

    FFTPanel::~FFTPanel() {
        analyzerRef.setON(false);
    }

    void FFTPanel::paint(juce::Graphics &g) {
        if (analyzerRef.getPreON() && !path1.isEmpty()) {
            g.setColour(uiBase.getColourByIdx(zlInterface::preColour));
            g.fillPath(path1);
        }

        if (analyzerRef.getPostON() && !path2.isEmpty()) {
            g.setColour(uiBase.getTextColor().withAlpha(0.5f));
            const auto thickness = uiBase.getFontSize() * 0.1f;
            g.strokePath(path2, juce::PathStrokeType(thickness, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));

            g.setColour(uiBase.getColourByIdx(zlInterface::postColour));
            g.fillPath(path2);
        }

        if (analyzerRef.getSideON() && !path3.isEmpty()) {
            g.setColour(uiBase.getColourByIdx(zlInterface::sideColour));
            g.fillPath(path3);
        }
    }

    void FFTPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        leftCorner = {bound.getX() * 0.9f, bound.getBottom() * 1.1f};
        rightCorner = {bound.getRight() * 1.1f, bound.getBottom() * 1.1f};
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());
        analyzerRef.setBound(bound);
    }

    void FFTPanel::updatePaths() {
        analyzerRef.updatePaths(path1, path2, path3);
        for (auto &path : {&path1, &path2, &path3}) {
            if (!path->isEmpty()) {
                path->lineTo(rightCorner);
                path->lineTo(leftCorner);
                path->closeSubPath();
            }
        }
    }
} // zlPanel
