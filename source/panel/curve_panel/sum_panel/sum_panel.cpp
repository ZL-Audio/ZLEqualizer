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
        std::array<bool, 5> useLRMS{false, false, false, false, false};
        constexpr std::array<zlDSP::lrType::lrTypes, 5> lrTypes{
            zlDSP::lrType::stereo, zlDSP::lrType::left, zlDSP::lrType::right, zlDSP::lrType::mid, zlDSP::lrType::side
        };
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            const auto idx = static_cast<size_t>(c.getFilterLRs(i));
            if (!c.getFilter(i).getBypass()) {
                useLRMS[idx] = true;
            }
        }

        for (size_t j = 0; j < useLRMS.size(); ++j) {
            if (!useLRMS[j]) { continue; }
            path.clear();

            c.updateDBs(lrTypes[j]);
            const auto dBs = c.getDBs();

            const auto bound = getLocalBounds().toFloat();
            for (size_t i = 0; i < zlIIR::frequencies.size(); ++i) {
                const auto x = static_cast<float>(i) / static_cast<float>(zlIIR::frequencies.size() - 1) * bound.
                               getWidth();
                const auto y = (dBs[i] / (-60) + 0.5f) * bound.getHeight();
                if (i == 0) {
                    path.startNewSubPath(x, y);
                } else {
                    path.lineTo(x, y);
                }
            }

            g.setColour(uiBase.getColorMap2(j));
            g.strokePath(path, juce::PathStrokeType(uiBase.getFontSize() * 0.2f,
                                                    juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    void SumPanel::timerCallback() {
        repaint();
    }
} // zlPanel
