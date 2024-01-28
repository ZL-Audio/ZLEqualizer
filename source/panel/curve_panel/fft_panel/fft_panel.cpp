// Copyright (C) 2023 - zsliu98
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
        path1.preallocateSpace(static_cast<int>(zlIIR::frequencies.size()) * 3 + 9);
        path2.preallocateSpace(static_cast<int>(zlIIR::frequencies.size()) * 3 + 9);
        setInterceptsMouseClicks(false, false);
        analyzerRef.setON(true);
    }

    FFTPanel::~FFTPanel() {
        analyzerRef.setON(false);
    }

    void FFTPanel::paint(juce::Graphics &g) {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());
        if (analyzerRef.getPreFFT().getIsFFTReady() && analyzerRef.getPostFFT().getIsFFTReady()) {
            path1.clear();
            analyzerRef.getPreFFT().createPath(path1, bound);
            analyzerRef.getPreFFT().resetDecay();
            path1.lineTo(getLocalBounds().getBottomRight().toFloat());
            path1.lineTo(getLocalBounds().getBottomLeft().toFloat());
            path1.closeSubPath();
            path2.clear();
            analyzerRef.getPostFFT().createPath(path2, bound);
            analyzerRef.getPostFFT().resetDecay();
            path2.lineTo(getLocalBounds().getBottomRight().toFloat());
            path2.lineTo(getLocalBounds().getBottomLeft().toFloat());
            path2.closeSubPath();
        }
        analyzerRef.getPreFFT().nextDecay();
        analyzerRef.getPostFFT().nextDecay();
        g.setColour(uiBase.getTextColor().withAlpha(0.1f));
        g.fillPath(path1);

        g.setColour(uiBase.getTextColor().withAlpha(0.5f));
        const auto thickness = uiBase.getFontSize() * 0.1f;
        g.strokePath(path2, juce::PathStrokeType(thickness, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
        g.setColour(uiBase.getTextColor().withAlpha(0.1f));
        g.fillPath(path2);
    }
} // zlPanel
