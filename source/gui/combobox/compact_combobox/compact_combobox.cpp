// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "compact_combobox.hpp"

namespace zlgui {
    CompactCombobox::CompactCombobox(const juce::String &label_text, const juce::StringArray &choices,
                                     UIBase &base, const multilingual::Labels label_idx,
                                     const std::vector<multilingual::Labels> &item_label_indices)
        : ui_base_(base),
          box_laf_(base) {
        juce::ignoreUnused(label_text);
        if (item_label_indices.size() < static_cast<size_t>(choices.size())) {
            combo_box_.addItemList(choices, 1);
        } else {
            const auto menu = combo_box_.getRootMenu();
            for (int i = 0; i < choices.size(); ++i) {
                juce::PopupMenu::Item item;
                item.itemID = i + 1;
                item.text = choices[i];
                item.tooltipText = ui_base_.getToolTipText(item_label_indices[static_cast<size_t>(i)]);
                item.isEnabled = true;
                item.isTicked = false;
                menu->addItem(item);
            }
        }

        combo_box_.setScrollWheelEnabled(false);
        combo_box_.setInterceptsMouseClicks(false, false);
        combo_box_.setLookAndFeel(&box_laf_);
        combo_box_.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(combo_box_);

        setEditable(true);

        if (label_idx != multilingual::Labels::kLabelNum) {
            SettableTooltipClient::setTooltip(ui_base_.getToolTipText(label_idx));
        }
    }


    CompactCombobox::~CompactCombobox() {
        // animator.cancelAllAnimations(false);
        combo_box_.setLookAndFeel(nullptr);
    }

    void CompactCombobox::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(),
                                            juce::jmin(bound.getHeight(), ui_base_.getFontSize() * 2.f));
        combo_box_.setBounds(bound.toNearestInt());
    }

    void CompactCombobox::mouseUp(const juce::MouseEvent &event) {
        combo_box_.mouseUp(event);
    }

    void CompactCombobox::mouseDown(const juce::MouseEvent &event) {
        combo_box_.mouseDown(event);
    }

    void CompactCombobox::mouseDrag(const juce::MouseEvent &event) {
        combo_box_.mouseDrag(event);
    }

    void CompactCombobox::mouseEnter(const juce::MouseEvent &event) {
        combo_box_.mouseEnter(event);
        box_laf_.setBoxAlpha(1.f);
        combo_box_.repaint();
    }

    void CompactCombobox::mouseExit(const juce::MouseEvent &event) {
        combo_box_.mouseExit(event);
        box_laf_.setBoxAlpha(0.f);
        combo_box_.repaint();
    }

    void CompactCombobox::mouseMove(const juce::MouseEvent &event) {
        combo_box_.mouseMove(event);
    }
}
