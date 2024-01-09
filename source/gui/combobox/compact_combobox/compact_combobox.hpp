// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef COMPACT_COMBOBOX_H
#define COMPACT_COMBOBOX_H

#include <friz/friz.h>

#include "compact_combobox_look_and_feel.hpp"

namespace zlInterface {
    class CompactCombobox : public juce::Component {
    public:
        CompactCombobox(const juce::String &labelText, const juce::StringArray &choices, UIBase &base);

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

        friz::Animator animator;
        static constexpr int animationId = 1;
    };
}


#endif //COMPACT_COMBOBOX_H
