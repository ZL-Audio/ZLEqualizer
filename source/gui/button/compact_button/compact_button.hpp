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
#include "compact_button_look_and_feel.hpp"

namespace zlInterface {
    class CompactButton final : public juce::Component {
    public:
        explicit CompactButton(const juce::String &labelText, UIBase &base,
                               multilingual::labels labelIdx = multilingual::labels::labelNum);

        ~CompactButton() override;

        void resized() override;

        void buttonDownAnimation();

        inline juce::ToggleButton &getButton() { return button; }

        inline void setEditable(const bool x) {
            lookAndFeel.setEditable(x);
            button.setInterceptsMouseClicks(x, false);
            repaint();
        }

        inline void setDrawable(juce::Drawable *x) { lookAndFeel.setDrawable(x); }

        CompactButtonLookAndFeel &getLAF() { return lookAndFeel; }

        void lookAndFeelChanged() override {
            lookAndFeel.updateImages();
        }

        void visibilityChanged() override {
            lookAndFeel.updateImages();
        }

    private:
        UIBase &uiBase;

        juce::ToggleButton button;
        CompactButtonLookAndFeel lookAndFeel;

        std::atomic<bool> fit = false;
    };
}
