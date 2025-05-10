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

namespace zlgui {
    class LeftRightCombobox final : public juce::Component,
                                    public juce::SettableTooltipClient {
    public:
        explicit LeftRightCombobox(const juce::StringArray &choices, UIBase &base,
                                   multilingual::Labels label_idx = multilingual::Labels::kLabelNum);

        ~LeftRightCombobox() override;

        void resized() override;

        inline juce::ComboBox &getBox() { return box_; }

        inline void setPadding(const float lr, const float ub) {
            lr_pad_ = lr;
            ub_pad_ = ub;
        }

        void selectLeft();

        void selectRight();

    private:
        UIBase &ui_base_;
        juce::DrawableButton left_button_, right_button_;
        LeftRightButtonLookAndFeel l_button_laf_, r_button_laf_;
        juce::ComboBox box_;
        LeftRightComboboxLookAndFeel laf_;

        float lr_pad_{0.f}, ub_pad_{0.f};
    };
} // zlgui
