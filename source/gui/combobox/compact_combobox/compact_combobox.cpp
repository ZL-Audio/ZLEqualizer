// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "compact_combobox.hpp"

namespace zlInterface {
    CompactCombobox::CompactCombobox(const juce::String &labelText, const juce::StringArray &choices,
                                     UIBase &base, const multilingual::labels labelIdx,
                                     const std::vector<multilingual::labels> &itemLabelIndices)
        : uiBase(base),
          boxLookAndFeel(base) {
        juce::ignoreUnused(labelText);
        if (itemLabelIndices.size() < static_cast<size_t>(choices.size())) {
            comboBox.addItemList(choices, 1);
        } else {
            const auto menu = comboBox.getRootMenu();
            for (int i = 0; i < choices.size(); ++i) {
                juce::PopupMenu::Item item;
                item.itemID = i + 1;
                item.text = choices[i];
                item.tooltipText = uiBase.getToolTipText(itemLabelIndices[static_cast<size_t>(i)]);
                item.isEnabled = true;
                item.isTicked = false;
                menu->addItem(item);
            }
        }

        comboBox.setScrollWheelEnabled(false);
        comboBox.setInterceptsMouseClicks(false, false);
        comboBox.setLookAndFeel(&boxLookAndFeel);
        comboBox.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(comboBox);

        setEditable(true);

        if (labelIdx != multilingual::labels::labelNum) {
            SettableTooltipClient::setTooltip(uiBase.getToolTipText(labelIdx));
        }
    }


    CompactCombobox::~CompactCombobox() {
        // animator.cancelAllAnimations(false);
        comboBox.setLookAndFeel(nullptr);
    }

    void CompactCombobox::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(),
                                            juce::jmin(bound.getHeight(), uiBase.getFontSize() * 2.f));
        comboBox.setBounds(bound.toNearestInt());
    }

    void CompactCombobox::mouseUp(const juce::MouseEvent &event) {
        comboBox.mouseUp(event);
    }

    void CompactCombobox::mouseDown(const juce::MouseEvent &event) {
        comboBox.mouseDown(event);
    }

    void CompactCombobox::mouseDrag(const juce::MouseEvent &event) {
        comboBox.mouseDrag(event);
    }

    void CompactCombobox::mouseEnter(const juce::MouseEvent &event) {
        comboBox.mouseEnter(event);
        boxLookAndFeel.setBoxAlpha(1.f);
        comboBox.repaint();
    }

    void CompactCombobox::mouseExit(const juce::MouseEvent &event) {
        comboBox.mouseExit(event);
        boxLookAndFeel.setBoxAlpha(0.f);
        comboBox.repaint();
    }

    void CompactCombobox::mouseMove(const juce::MouseEvent &event) {
        comboBox.mouseMove(event);
    }
}
