// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_panel.hpp"

namespace zlPanel {
    MatchPanel::MatchPanel(zlEqMatch::EqMatchAnalyzer<double> &analyzer, zlInterface::UIBase &base)
        : uiBase(base) {
        juce::ignoreUnused(analyzer, uiBase);
        setInterceptsMouseClicks(true, false);
    }

    MatchPanel::~MatchPanel() = default;
} // zlPanel
