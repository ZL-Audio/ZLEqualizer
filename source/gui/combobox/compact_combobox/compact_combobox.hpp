// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "compact_combobox_look_and_feel.hpp"

namespace zlgui {
    class CompactCombobox final : public juce::Component,
                                  public juce::SettableTooltipClient {
    public:
        CompactCombobox(const juce::String &label_text, const juce::StringArray &choices, UIBase &base,
                        multilingual::Labels label_idx = multilingual::Labels::kLabelNum,
                        const std::vector<multilingual::Labels> &item_label_indices = {});

        ~CompactCombobox() override;

        void resized() override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseEnter(const juce::MouseEvent &event) override;

        void mouseExit(const juce::MouseEvent &event) override;

        void mouseMove(const juce::MouseEvent &event) override;

        inline void setEditable(const bool x) {
            box_laf_.setEditable(x);
            setInterceptsMouseClicks(x, false);
        }

        inline juce::ComboBox &getBox() { return combo_box_; }

        inline CompactComboboxLookAndFeel &getLAF() { return box_laf_; }

    private:
        zlgui::UIBase &ui_base_;
        CompactComboboxLookAndFeel box_laf_;
        juce::ComboBox combo_box_;
    };
}
