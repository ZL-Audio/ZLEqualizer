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
#include "../interface_definitions.hpp"
#include "../combobox/combobox.hpp"

namespace zlgui {
    class ColourMapSelector final : public juce::Component,
                                    private juce::ComboBox::Listener {
    public:
        explicit ColourMapSelector(zlgui::UIBase &base, float box_width = .5f);

        void paint(juce::Graphics &g) override;

        void resized() override;

        juce::ComboBox &getBox() { return map_box_.getBox(); }

    private:
        zlgui::UIBase &ui_base_;
        zlgui::CompactCombobox map_box_;
        float map_box_width_p_{.5f};

        void comboBoxChanged(juce::ComboBox *combo_box_that_has_changed) override;
    };
} // zlgui
