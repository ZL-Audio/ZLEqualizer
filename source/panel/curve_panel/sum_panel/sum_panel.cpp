// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "sum_panel.hpp"

namespace zlPanel {
    SumPanel::SumPanel(zlInterface::UIBase &base, zlDSP::Controller<float> &controller)
        : uiBase(base), c(controller) {
        path.preallocateSpace(zlIIR::frequencies.size() * 3);
        startTimerHz(60);
    }

    SumPanel::~SumPanel() {
        path.clear();
        stopTimer();
    }

    void SumPanel::paint(juce::Graphics &g) {
        path.clear();
        // dBs.fill(0.f);
        c.updateDBs();
        c.setDBs(dBs);

        const auto bound = getLocalBounds().toFloat();
        for (size_t i = 0; i < zlIIR::frequencies.size(); ++i) {
            const auto x = static_cast<float>(i) / static_cast<float>(zlIIR::frequencies.size() - 1) * bound.getWidth();
            const auto y = (dBs[i] / (-60) + 0.5f) * bound.getHeight();
            if (i == 0) {
                path.startNewSubPath(x, y);
            } else {
                path.lineTo(x, y);
            }
        }

        g.setColour(uiBase.getExtraColor1());
        g.strokePath(path, juce::PathStrokeType(uiBase.getFontSize() * 0.25f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    void SumPanel::timerCallback() {
        repaint();
    }
} // zlPanel
