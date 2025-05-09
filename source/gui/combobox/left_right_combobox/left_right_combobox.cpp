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
                                         const multilingual::labels labelIdx)
        : uiBase(base),
          leftButton("", juce::DrawableButton::ButtonStyle::ImageFitted),
          rightButton("", juce::DrawableButton::ButtonStyle::ImageFitted),
          lLAF(base), rLAF(base), lookAndFeel(base) {
        box.addItemList(choices, 1);
        box.setScrollWheelEnabled(false);
        box.setLookAndFeel(&lookAndFeel);
        box.setInterceptsMouseClicks(false, false);

        lLAF.setDirection(0.5f);
        leftButton.setLookAndFeel(&lLAF);
        rLAF.setDirection(0.0f);
        rightButton.setLookAndFeel(&rLAF);
        leftButton.onClick = [this]() { selectLeft(); };
        rightButton.onClick = [this]() { selectRight(); };

        addAndMakeVisible(box);
        addAndMakeVisible(leftButton);
        addAndMakeVisible(rightButton);

        if (labelIdx != multilingual::labels::labelNum) {
            SettableTooltipClient::setTooltip(uiBase.getToolTipText(labelIdx));
        }
    }

    LeftRightCombobox::~LeftRightCombobox() {
        box.setLookAndFeel(nullptr);
        leftButton.setLookAndFeel(nullptr);
        rightButton.setLookAndFeel(nullptr);
    }

    void LeftRightCombobox::resized() {
        auto tempBound = getLocalBounds().toFloat();
        tempBound = tempBound.withSizeKeepingCentre(tempBound.getWidth() - lrPad,
                                            uiBase.getFontSize() - ubPad);
        auto bound = tempBound.toNearestInt().constrainedWithin(getLocalBounds());
        const auto buttonSize = static_cast<int>(uiBase.getFontSize());
        const auto leftBound = bound.removeFromLeft(buttonSize);
        const auto rightBound = bound.removeFromRight(buttonSize);
        leftButton.setBounds(leftBound);
        rightButton.setBounds(rightBound);
        box.setBounds(bound);
    }

    void LeftRightCombobox::selectLeft() {
        if (box.getSelectedId() <= 1) {
            box.setSelectedId(box.getNumItems());
        } else {
            box.setSelectedId(box.getSelectedId() - 1);
        }
    }

    void LeftRightCombobox::selectRight() {
        if (box.getSelectedId() == box.getNumItems()) {
            box.setSelectedId(1);
        } else {
            box.setSelectedId(box.getSelectedId() + 1);
        }
    }
} // zlgui
