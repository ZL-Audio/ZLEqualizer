// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "left_right_combobox.hpp"

namespace zlgui {
    LeftRightCombobox::LeftRightCombobox(const juce::StringArray &choices, UIBase &base,
                                         const multilingual::Labels label_idx)
        : ui_base_(base),
          left_button_("", juce::DrawableButton::ButtonStyle::ImageFitted),
          right_button_("", juce::DrawableButton::ButtonStyle::ImageFitted),
          l_button_laf_(base), r_button_laf_(base), laf_(base) {
        box_.addItemList(choices, 1);
        box_.setScrollWheelEnabled(false);
        box_.setLookAndFeel(&laf_);
        box_.setInterceptsMouseClicks(false, false);

        l_button_laf_.setDirection(0.5f);
        left_button_.setLookAndFeel(&l_button_laf_);
        r_button_laf_.setDirection(0.0f);
        right_button_.setLookAndFeel(&r_button_laf_);
        left_button_.onClick = [this]() { selectLeft(); };
        right_button_.onClick = [this]() { selectRight(); };

        addAndMakeVisible(box_);
        addAndMakeVisible(left_button_);
        addAndMakeVisible(right_button_);

        if (label_idx != multilingual::Labels::kLabelNum) {
            SettableTooltipClient::setTooltip(ui_base_.getToolTipText(label_idx));
        }
    }

    LeftRightCombobox::~LeftRightCombobox() {
        box_.setLookAndFeel(nullptr);
        left_button_.setLookAndFeel(nullptr);
        right_button_.setLookAndFeel(nullptr);
    }

    void LeftRightCombobox::resized() {
        auto temp_bound = getLocalBounds().toFloat();
        temp_bound = temp_bound.withSizeKeepingCentre(temp_bound.getWidth() - lr_pad_,
                                            ui_base_.getFontSize() - ub_pad_);
        auto bound = temp_bound.toNearestInt().constrainedWithin(getLocalBounds());
        const auto button_size = static_cast<int>(ui_base_.getFontSize());
        const auto left_bound = bound.removeFromLeft(button_size);
        const auto right_bound = bound.removeFromRight(button_size);
        left_button_.setBounds(left_bound);
        right_button_.setBounds(right_bound);
        box_.setBounds(bound);
    }

    void LeftRightCombobox::selectLeft() {
        if (box_.getSelectedId() <= 1) {
            box_.setSelectedId(box_.getNumItems());
        } else {
            box_.setSelectedId(box_.getSelectedId() - 1);
        }
    }

    void LeftRightCombobox::selectRight() {
        if (box_.getSelectedId() == box_.getNumItems()) {
            box_.setSelectedId(1);
        } else {
            box_.setSelectedId(box_.getSelectedId() + 1);
        }
    }
} // zlgui
