// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "conflict_panel.hpp"

namespace zlPanel {
    ConflictPanel::ConflictPanel(zlFFT::ConflictAnalyzer<double> &conflictAnalyzer, zlInterface::UIBase &base)
        : analyzer(conflictAnalyzer), uiBase(base) {
        setInterceptsMouseClicks(false, false);
        juce::ignoreUnused(uiBase);
    }

    ConflictPanel::~ConflictPanel() = default;

    void ConflictPanel::paint(juce::Graphics &g) {
        if (!analyzer.getON()) { return; }
        const auto bound = getLocalBounds().toFloat();
        analyzer.drawGradient(g, bound);
    }

    void ConflictPanel::resized() {
        analyzer.setLeftRight(0.f, static_cast<float>(getRight()));
    }
} // zlPanel
