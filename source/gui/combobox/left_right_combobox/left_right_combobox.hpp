// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_LEFT_RIGHT_COMBOBOX_HPP
#define ZLEqualizer_LEFT_RIGHT_COMBOBOX_HPP

#include "left_right_combobox_look_and_feel.hpp"
#include "left_right_button_look_and_feel.hpp"

namespace zlInterface {
    class LeftRightCombobox final : public juce::Component {
    public:
        explicit LeftRightCombobox(const juce::StringArray &choices, UIBase &base);

        ~LeftRightCombobox() override;

        void resized() override;

        inline juce::ComboBox& getBox() {return box;}

        inline void setPadding(const float lr, const float ub) {
            lrPad.store(lr);
            ubPad.store(ub);
        }

        void selectLeft();

        void selectRight();

    private:
        UIBase &uiBase;
        juce::DrawableButton leftButton, rightButton;
        LeftRightButtonLookAndFeel lLAF, rLAF;
        juce::ComboBox box;
        LeftRightComboboxLookAndFeel lookAndFeel;

        std::atomic<float> lrPad = 0, ubPad = 0;
    };
} // zlInterface

#endif //ZLEqualizer_LEFT_RIGHT_COMBOBOX_HPP
