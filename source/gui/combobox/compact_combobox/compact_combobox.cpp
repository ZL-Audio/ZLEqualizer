//
// Created by Zishu Liu on 12/30/23.
//

#include "compact_combobox.h"

namespace zlInterface {
    CompactCombobox::CompactCombobox(const juce::String &labelText, const juce::StringArray &choices,
                                     UIBase &base) : boxLookAndFeel(base) {
        juce::ignoreUnused(labelText);
        comboBox.addItemList(choices, 1);
        comboBox.setLookAndFeel(&boxLookAndFeel);
        comboBox.setScrollWheelEnabled(false);
        addAndMakeVisible(comboBox);
    }


    CompactCombobox::~CompactCombobox() {
        comboBox.setLookAndFeel(nullptr);
    }

    void CompactCombobox::resized() {
        comboBox.setBounds(getLocalBounds());
    }


}
