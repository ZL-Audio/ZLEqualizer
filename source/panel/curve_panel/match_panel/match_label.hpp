// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../gui/gui.hpp"

namespace zlpanel {
    class MatchLabel : public juce::Component {
    public:
        explicit MatchLabel(zlgui::UIBase &base);

        void paint(juce::Graphics &g) override;

        void resized() override;

    private:
        zlgui::UIBase &ui_base_;
        zlgui::NameLookAndFeel label_laf_;
        juce::Label running_label_;
    };
} // zlpanel
