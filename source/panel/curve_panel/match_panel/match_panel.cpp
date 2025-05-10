// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_panel.hpp"

namespace zlpanel {
    MatchPanel::MatchPanel(zldsp::eq_match::EqMatchAnalyzer<double> &analyzer,
                           juce::AudioProcessorValueTreeState &parameters_NA, zlgui::UIBase &base)
        : ui_base_(base), match_analyzer_panel_(analyzer, parameters_NA, base) {
        juce::ignoreUnused(analyzer, ui_base_);
        setInterceptsMouseClicks(true, false);
        addChildComponent(match_analyzer_panel_);
    }

    MatchPanel::~MatchPanel() = default;

    void MatchPanel::resized() {
        match_analyzer_panel_.setBounds(getLocalBounds());
    }
} // zlpanel
