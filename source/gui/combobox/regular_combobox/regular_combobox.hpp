// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef ZL_REGULAR_COMBOBOX_COMPONENT_H
#define ZL_REGULAR_COMBOBOX_COMPONENT_H

#include "regular_combobox_look_and_feel.hpp"
#include "../../label/name_look_and_feel.hpp"

namespace zlInterface {
    class RegularCombobox : public juce::Component {
    public:
        explicit RegularCombobox(const juce::String &labelText, const juce::StringArray &choices, UIBase &base) :
                myLookAndFeel(base), nameLookAndFeel(base) {
            uiBase = &base;
            comboBox.addItemList(choices, 1);
            comboBox.setLookAndFeel(&myLookAndFeel);
            comboBox.setScrollWheelEnabled(false);
            addAndMakeVisible(comboBox);
            label.setText(labelText, juce::dontSendNotification);
            label.setLookAndFeel(&nameLookAndFeel);
            addAndMakeVisible(label);

            uiBase = &base;
        }

        ~RegularCombobox() override {
            comboBox.setLookAndFeel(nullptr);
            label.setLookAndFeel(nullptr);
        }

        void resized() override {
            auto bound = getLocalBounds().toFloat();
            auto labelBound = bound.removeFromTop(labelHeight * bound.getHeight());
            label.setBounds(labelBound.toNearestInt());
            comboBox.setBounds(bound.toNearestInt());
        }

        void paint(juce::Graphics &g) override {
            juce::ignoreUnused(g);
        }

        juce::ComboBox &getComboBox() { return comboBox; }

        juce::Label &getLabel() { return label; }

        void setEditable(bool f) {
            myLookAndFeel.setEditable(f);
            nameLookAndFeel.setEditable(f);
        }

    private:
        RegularComboboxLookAndFeel myLookAndFeel;
        NameLookAndFeel nameLookAndFeel;
        juce::ComboBox comboBox;
        juce::Label label;

        constexpr static float boxHeight = 0.7f;
        constexpr static float labelHeight = 1.f - boxHeight;
        constexpr static float boxRatio = 0.45f;

        UIBase *uiBase;
    };
}
#endif //ZL_REGULAR_COMBOBOX_COMPONENT_H
