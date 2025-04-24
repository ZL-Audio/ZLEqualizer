// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "conflict_panel.hpp"

namespace zlPanel {
    ConflictPanel::ConflictPanel(zlFFTAnalyzer::ConflictAnalyzer<double> &conflictAnalyzer, zlInterface::UIBase &base)
        : analyzer(conflictAnalyzer), uiBase(base) {
        analyzer.start();
        setInterceptsMouseClicks(false, false);
        juce::ignoreUnused(uiBase);
    }

    ConflictPanel::~ConflictPanel() {
        analyzer.stop();
    }

    void ConflictPanel::paint(juce::Graphics &g) {
        g.setGradientFill(gradient);
        g.fillRect(getLocalBounds());
    }

    void ConflictPanel::resized() {
        analyzer.setLeftRight(0.f, static_cast<float>(getRight()));
    }
} // zlPanel
