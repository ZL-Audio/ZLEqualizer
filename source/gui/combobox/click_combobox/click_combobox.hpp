// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../compact_combobox/compact_combobox.hpp"
#include "../../label/name_look_and_feel.hpp"
#include "click_combobox_button_look_and_feel.hpp"

namespace zlgui {
    class ClickCombobox final : public juce::Component {
    public:
        enum LabelPos {
            kLeft, kRight, kTop, kBottom
        };

        ClickCombobox(const juce::String &label_text, const juce::StringArray &choices, UIBase &base,
                      multilingual::Labels label_idx = multilingual::Labels::kLabelNum);

        ~ClickCombobox() override;

        void resized() override;

        void setLabelScale(const float x) { l_scale_.store(x); }

        void setLabelPos(const LabelPos x) { l_pos_.store(x); }

        ClickComboboxButtonLookAndFeel &getLabelLAF() { return label_laf_; }

        CompactCombobox &getCompactBox() { return compact_box_; }

    private:
        CompactCombobox compact_box_;
        juce::DrawableButton label_;
        ClickComboboxButtonLookAndFeel label_laf_;
        std::atomic<float> l_scale_{0.f};
        std::atomic<LabelPos> l_pos_{kLeft};

        void selectRight();
    };
} // zlgui
