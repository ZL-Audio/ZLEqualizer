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

#include "../../interface_definitions.hpp"

namespace zlgui::combobox {
    class ClickCombobox final : public juce::Component,
                                public juce::SettableTooltipClient,
                                private juce::ComboBox::Listener {
    public:
        ClickCombobox(UIBase& base, const std::vector<std::string>& choices, const juce::String& tooltip_text = "",
                      const std::vector<juce::String>& item_labels = {});

        ClickCombobox(UIBase& base, const std::vector<juce::Drawable*>& choices,
                      const juce::String& tooltip_text = "",
                      const std::vector<juce::String>& item_labels = {});

        ~ClickCombobox() override;

        void paint(juce::Graphics& g) override;

        juce::ComboBox& getBox() { return combo_box_; }

        void setSizeScale(float width_scale, float height_scale);

    private:
        UIBase& base_;
        juce::ComboBox combo_box_;
        std::vector<juce::String> item_labels_;

        std::vector<std::string> choice_text_;
        std::vector<juce::Drawable*> choice_icons_;

        float width_scale_{1.f}, height_scale_{1.f};

        void mouseDown(const juce::MouseEvent& event) override;

        void comboBoxChanged(juce::ComboBox*) override;
    };
} // zlgui
