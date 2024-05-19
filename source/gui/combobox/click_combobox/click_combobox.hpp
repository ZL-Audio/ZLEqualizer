// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_CLICK_COMBOBOX_HPP
#define ZLEqualizer_CLICK_COMBOBOX_HPP

#include "../compact_combobox/compact_combobox.hpp"
#include "../../label/name_look_and_feel.hpp"
#include "click_combobox_button_look_and_feel.hpp"

namespace zlInterface {
    class ClickCombobox final : public juce::Component {
    public:
        enum LabelPos {
            left, right, top, bottom
        };

        ClickCombobox(const juce::String &labelText, const juce::StringArray &choices, UIBase &base);

        ~ClickCombobox() override;

        void resized() override;

        void setLabelScale(const float x) { lScale.store(x); }

        void setLabelPos(const LabelPos x) { lPos.store(x); }

        ClickComboboxButtonLookAndFeel &getLabelLAF() { return labelLAF; }

        CompactCombobox &getCompactBox() { return compactBox; }

    private:
        CompactCombobox compactBox;
        juce::DrawableButton label;
        ClickComboboxButtonLookAndFeel labelLAF;
        std::atomic<float> lScale{0.f};
        std::atomic<LabelPos> lPos{left};

        void selectRight();
    };
} // zlInterface

#endif //ZLEqualizer_CLICK_COMBOBOX_HPP
