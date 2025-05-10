// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "click_combobox.hpp"

namespace zlgui {
    ClickCombobox::ClickCombobox(const juce::String &label_text, const juce::StringArray &choices,
                                 UIBase &base,
                                 const multilingual::Labels label_idx)
        : compact_box_("", choices, base),
          label_("", juce::DrawableButton::ButtonStyle::ImageFitted),
          label_laf_(base, label_text) {
        addAndMakeVisible(compact_box_);
        label_laf_.setJustification(juce::Justification::centredRight);
        label_.setLookAndFeel(&label_laf_);
        label_.onClick = [this]() { selectRight(); };
        addAndMakeVisible(label_);

        if (label_idx != multilingual::Labels::kLabelNum) {
            compact_box_.setTooltip(base.getToolTipText(label_idx));
            label_.setTooltip(base.getToolTipText(label_idx));
        }
    }

    ClickCombobox::~ClickCombobox() {
        label_.setLookAndFeel(nullptr);
    }

    void ClickCombobox::selectRight() {
        auto &box = compact_box_.getBox();
        if (box.getSelectedId() == box.getNumItems()) {
            box.setSelectedId(1);
        } else {
            box.setSelectedId(box.getSelectedId() + 1);
        }
    }

    void ClickCombobox::resized() {
        auto bound = getLocalBounds().toFloat();
        const auto scale = l_scale_.load();
        switch (l_pos_.load()) {
            case kLeft: {
                label_.setBounds(bound.removeFromLeft(scale * bound.getWidth()).toNearestInt());
                break;
            }
            case kRight: {
                label_.setBounds(bound.removeFromRight(scale * bound.getWidth()).toNearestInt());
                break;
            }
            case kTop: {
                label_.setBounds(bound.removeFromTop(scale * bound.getHeight()).toNearestInt());
                break;
            }
            case kBottom: {
                label_.setBounds(bound.removeFromBottom(scale * bound.getHeight()).toNearestInt());
                break;
            }
        }
        compact_box_.setBounds(bound.toNearestInt());
    }
} // zlgui
