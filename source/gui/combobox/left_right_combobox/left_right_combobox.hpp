// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "left_right_combobox_look_and_feel.hpp"
#include "left_right_button_look_and_feel.hpp"

namespace zlInterface {
    class LeftRightCombobox final : public juce::Component,
                                    public juce::SettableTooltipClient {
    public:
        explicit LeftRightCombobox(const juce::StringArray &choices, UIBase &base,
                                   multilingual::labels labelIdx = multilingual::labels::labelNum);

        ~LeftRightCombobox() override;

        void resized() override;

        inline juce::ComboBox &getBox() { return box; }

        inline void setPadding(const float lr, const float ub) {
            lrPad = lr;
            ubPad = ub;
        }

        void selectLeft();

        void selectRight();

    private:
        UIBase &uiBase;
        juce::DrawableButton leftButton, rightButton;
        LeftRightButtonLookAndFeel lLAF, rLAF;
        juce::ComboBox box;
        LeftRightComboboxLookAndFeel lookAndFeel;

        float lrPad{0.f}, ubPad{0.f};
    };
} // zlInterface
