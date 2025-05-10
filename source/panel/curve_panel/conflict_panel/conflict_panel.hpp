// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"

namespace zlpanel {
    class ConflictPanel final : public juce::Component {
    public:
        explicit ConflictPanel(zldsp::analyzer::ConflictAnalyzer<double> &conflictAnalyzer,
                               zlgui::UIBase &base);

        ~ConflictPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void updateGradient() {
            if (analyzer_ref_.getON()) {
                analyzer_ref_.updateGradient(gradient_);
                setVisible(true);
            } else {
                setVisible(false);
            }
        }

    private:
        zldsp::analyzer::ConflictAnalyzer<double> &analyzer_ref_;
        zlgui::UIBase &ui_base_;
        juce::Path path_;
        juce::ColourGradient gradient_;
    };
} // zlpanel
