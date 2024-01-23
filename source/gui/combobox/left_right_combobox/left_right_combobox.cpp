// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "left_right_combobox.hpp"

namespace zlInterface {
    LeftRightCombobox::LeftRightCombobox(const juce::StringArray &choices, UIBase &base)
        : uiBase(base),
    leftButton("", juce::DrawableButton::ButtonStyle::ImageFitted),
    rightButton("", juce::DrawableButton::ButtonStyle::ImageFitted),
    lLAF(base), rLAF(base),  lookAndFeel(base) {
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

        setInterceptsMouseClicks(false, true);

        addAndMakeVisible(box);
        addAndMakeVisible(leftButton);
        addAndMakeVisible(rightButton);
    }

    LeftRightCombobox::~LeftRightCombobox() {
        box.setLookAndFeel(nullptr);
        leftButton.setLookAndFeel(nullptr);
        rightButton.setLookAndFeel(nullptr);
    }

    void LeftRightCombobox::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() - lrPad.load(),
                                            uiBase.getFontSize() - ubPad.load());
        const auto leftBound = bound.removeFromLeft(uiBase.getFontSize());
        const auto rightBound = bound.removeFromRight(uiBase.getFontSize());
        leftButton.setBounds(leftBound.toNearestInt());
        rightButton.setBounds(rightBound.toNearestInt());
        box.setBounds(bound.toNearestInt());
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
} // zlInterface
