// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "click_combobox.hpp"

namespace zlInterface {
    ClickCombobox::ClickCombobox(const juce::String &labelText, const juce::StringArray &choices, UIBase &base)
        : compactBox("", choices, base),
          label("", juce::DrawableButton::ButtonStyle::ImageFitted),
          labelLAF(base, labelText) {
        addAndMakeVisible(compactBox);
        labelLAF.setJustification(juce::Justification::centredRight);
        label.setLookAndFeel(&labelLAF);
        label.onClick = [this]() { selectRight(); };
        addAndMakeVisible(label);
    }

    ClickCombobox::~ClickCombobox() {
        label.setLookAndFeel(nullptr);
    }

    void ClickCombobox::selectRight() {
        auto &box = compactBox.getBox();
        if (box.getSelectedId() == box.getNumItems()) {
            box.setSelectedId(1);
        } else {
            box.setSelectedId(box.getSelectedId() + 1);
        }
    }

    void ClickCombobox::resized() {
        auto bound = getLocalBounds().toFloat();
        const auto scale = lScale.load();
        switch (lPos.load()) {
            case left: {
                label.setBounds(bound.removeFromLeft(scale * bound.getWidth()).toNearestInt());
                break;
            }
            case right: {
                label.setBounds(bound.removeFromRight(scale * bound.getWidth()).toNearestInt());
                break;
            }
            case top: {
                label.setBounds(bound.removeFromTop(scale * bound.getHeight()).toNearestInt());
                break;
            }
            case bottom: {
                label.setBounds(bound.removeFromBottom(scale * bound.getHeight()).toNearestInt());
                break;
            }
        }
        compactBox.setBounds(bound.toNearestInt());
    }
} // zlInterface
