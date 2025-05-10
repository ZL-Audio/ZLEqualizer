// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "match_analyzer_panel.hpp"

namespace zlpanel {
    class MatchPanel final : public juce::Component {
    public:
        explicit MatchPanel(zldsp::eq_match::EqMatchAnalyzer<double> &analyzer,
                            juce::AudioProcessorValueTreeState &parameters_NA,
                            zlgui::UIBase &base);

        ~MatchPanel() override;

        void resized() override;

        void updatePaths() {
            match_analyzer_panel_.updatePaths();
        }

        void updateDraggers() {
            match_analyzer_panel_.updateDraggers();
        }

        void visibilityChanged() override {
            match_analyzer_panel_.setVisible(isVisible());
        }

    private:
        zlgui::UIBase &ui_base_;
        MatchAnalyzerPanel match_analyzer_panel_;
    };
} // zlpanel
