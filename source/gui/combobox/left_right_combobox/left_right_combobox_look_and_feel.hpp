// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_LEFT_RIGHT_COMBOBOX_LOOK_AND_FEEL_HPP
#define ZLEqualizer_LEFT_RIGHT_COMBOBOX_LOOK_AND_FEEL_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"


namespace zlInterface {
    class LeftRightComboboxLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        // rounded menu box
        explicit LeftRightComboboxLookAndFeel(UIBase &base) {
            uiBase = &base;
        }

        void drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown, int, int, int, int,
                          juce::ComboBox &box) override {
            juce::ignoreUnused(g, width, height, isButtonDown, box);
        }

        void positionComboBoxText(juce::ComboBox &box, juce::Label &label) override {
            label.setBounds(box.getLocalBounds());
        }

        void drawLabel(juce::Graphics &g, juce::Label &label) override {
            if (editable.load()) {
                g.setColour(uiBase->getTextColor());
            } else {
                g.setColour(uiBase->getTextInactiveColor());
            }
            g.setFont(uiBase->getFontSize() * fontScale);
            g.drawText(label.getText(), label.getLocalBounds(), juce::Justification::centred);
        }

        int getMenuWindowFlags() override {
            return 1;
        }

        inline void setEditable(bool f) { editable.store(f); }

        inline void setBoxAlpha(const float x) { boxAlpha.store(x); }

        inline float getBoxAlpha() const { return boxAlpha.load(); }

    private:
        std::atomic<bool> editable = true;
        std::atomic<float> boxAlpha;
        static constexpr float fontScale = 1.5f;

        UIBase *uiBase;
    };
}

#endif //ZLEqualizer_LEFT_RIGHT_COMBOBOX_LOOK_AND_FEEL_HPP
