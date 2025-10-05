// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

//
// Created by Zishu Liu on 7/24/25.
//

#pragma once

#include "click_text_button_laf.hpp"

namespace zlgui::button {
    class ClickTextButton final : public juce::Component {
    public:
        explicit ClickTextButton(UIBase& base,
                                 const juce::String& text = "",
                                 const juce::String& tooltip_text = "")
            : look_and_feel_(base) {
            button_.setButtonText(text);
            button_.setLookAndFeel(&look_and_feel_);

            addAndMakeVisible(button_);

            if (tooltip_text.length() > 0) {
                button_.setTooltip(tooltip_text);
            }

            setInterceptsMouseClicks(false, true);
        }

        ~ClickTextButton() override = default;

        void resized() override {
            button_.setBounds(getLocalBounds());
        }

        juce::TextButton& getButton() { return button_; }

        ClickTextButtonLookAndFeel& getLAF() { return look_and_feel_; }

    private:
        ClickTextButtonLookAndFeel look_and_feel_;
        juce::TextButton button_;
    };
} // zlgui
