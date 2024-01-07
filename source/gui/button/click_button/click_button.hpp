// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_CLICK_BUTTON_HPP
#define ZLEqualizer_CLICK_BUTTON_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "click_button_look_and_feel.hpp"

namespace zlInterface {
    class ClickButton final : public juce::Component {
    public:
        explicit ClickButton(juce::Drawable *image, UIBase &base);

        ~ClickButton() override;

        void resized() override;

        inline juce::DrawableButton &getButton() { return button; }

        inline ClickButtonLookAndFeel &getLookAndFeel() { return lookAndFeel; }

    private:
        juce::DrawableButton button;
        ClickButtonLookAndFeel lookAndFeel;
    };
} // zlInterface

#endif //ZLEqualizer_CLICK_BUTTON_HPP
