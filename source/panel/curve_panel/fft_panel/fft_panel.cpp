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
        path3.preallocateSpace(static_cast<int>(zlIIR::frequencies.size()) * 3 + 9);
        setInterceptsMouseClicks(false, false);
        analyzerRef.setON(true);
    }

    FFTPanel::~FFTPanel() {
        analyzerRef.setON(false);
    }

    void FFTPanel::paint(juce::Graphics &g) {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());
        for (auto &fft : {&analyzerRef.getPreFFT(), &analyzerRef.getPostFFT(), &analyzerRef.getSideFFT()}) {
            fft->setExtraTilt(uiBase.getFFTExtraTilt());
            fft->setExtraSpeed(uiBase.getFFTExtraSpeed());
        }
        if (analyzerRef.getPreON()) {
            auto &fft{analyzerRef.getPreFFT()};
            auto &path{path1};

            path.clear();
            fft.createPath(path, bound);
            path.lineTo(getLocalBounds().getBottomRight().toFloat());
            path.lineTo(getLocalBounds().getBottomLeft().toFloat());
            path.closeSubPath();

            g.setColour(uiBase.getColourByIdx(zlInterface::preColour));
            g.fillPath(path);
        }

        if (analyzerRef.getPostON()) {
            auto &fft{analyzerRef.getPostFFT()};
            auto &path{path2};
            path.clear();
            fft.createPath(path, bound);

            g.setColour(uiBase.getTextColor().withAlpha(0.5f));
            const auto thickness = uiBase.getFontSize() * 0.1f;
            g.strokePath(path, juce::PathStrokeType(thickness, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));

            path.lineTo(getLocalBounds().getBottomRight().toFloat());
            path.lineTo(getLocalBounds().getBottomLeft().toFloat());
            path.closeSubPath();

            g.setColour(uiBase.getColourByIdx(zlInterface::postColour));
            g.fillPath(path);
        }

        if (analyzerRef.getSideON()) {
            auto &fft{analyzerRef.getSideFFT()};
            auto &path{path3};

            path.clear();
            fft.createPath(path, bound);
            path.lineTo(getLocalBounds().getBottomRight().toFloat());
            path.lineTo(getLocalBounds().getBottomLeft().toFloat());
            path.closeSubPath();

            g.setColour(uiBase.getColourByIdx(zlInterface::sideColour));
            g.fillPath(path);
        }
    }
} // zlPanel
