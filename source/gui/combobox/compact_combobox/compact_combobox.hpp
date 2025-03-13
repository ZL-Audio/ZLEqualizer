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

namespace zlInterface {
    class CompactCombobox final : public juce::Component,
                                  public juce::SettableTooltipClient {
    public:
        CompactCombobox(const juce::String &labelText, const juce::StringArray &choices, UIBase &base,
                        multilingual::labels labelIdx = multilingual::labels::labelNum,
                        const std::vector<multilingual::labels> &itemLabelIndices = {});

        ~CompactCombobox() override;

        void resized() override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseEnter(const juce::MouseEvent &event) override;

        void mouseExit(const juce::MouseEvent &event) override;

        void mouseMove(const juce::MouseEvent &event) override;

        inline void setEditable(const bool x) {
            boxLookAndFeel.setEditable(x);
            setInterceptsMouseClicks(x, false);
        }

        inline juce::ComboBox &getBox() { return comboBox; }

        inline CompactComboboxLookAndFeel &getLAF() { return boxLookAndFeel; }

    private:
        zlInterface::UIBase &uiBase;
        CompactComboboxLookAndFeel boxLookAndFeel;
        juce::ComboBox comboBox;
    };
}
