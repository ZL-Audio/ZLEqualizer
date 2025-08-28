// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "click_combobox.hpp"

namespace zlgui::combobox {
    ClickCombobox::ClickCombobox(UIBase &base, const std::vector<std::string> &choices,
                                 const juce::String &tooltip_text,
                                 const std::vector<juce::String> &item_labels)
        : base_(base) {
        choice_text_ = choices;
        if (tooltip_text.length() > 0) {
            SettableTooltipClient::setTooltip(tooltip_text);
        } else if (item_labels.size() >= choices.size()) {
            item_labels_ = item_labels;
            SettableTooltipClient::setTooltip(item_labels_[0]);
        }
        for (size_t i = 0; i < choice_text_.size(); ++i) {
            combo_box_.addItem("t", static_cast<int>(i + 1));
        }
        combo_box_.addListener(this);
    }

    ClickCombobox::ClickCombobox(UIBase &base, const std::vector<juce::Drawable *> &choices,
                                 const juce::String &tooltip_text,
                                 const std::vector<juce::String> &item_labels)
        : base_(base) {
        choice_icons_ = choices;
        if (tooltip_text.length() > 0) {
            SettableTooltipClient::setTooltip(tooltip_text);
        } else if (item_labels.size() >= choices.size()) {
            item_labels_ = item_labels;
            SettableTooltipClient::setTooltip(item_labels_[0]);
        }
        for (size_t i = 0; i < choice_icons_.size(); ++i) {
            combo_box_.addItem("t", static_cast<int>(i + 1));
        }
        combo_box_.addListener(this);
    }

    ClickCombobox::~ClickCombobox() {
        combo_box_.removeListener(this);
    }

    void ClickCombobox::paint(juce::Graphics &g) {
        if (combo_box_.getSelectedItemIndex() < 0) return;
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() * width_scale_, bound.getHeight() * height_scale_);
        const auto choice_idx = static_cast<size_t>(combo_box_.getSelectedItemIndex());
        if (!choice_text_.empty()) {
            const auto text = choice_text_[choice_idx];
            g.setColour(base_.getTextColor());
            g.drawText(text, bound, juce::Justification::centred);
        }
        if (!choice_icons_.empty()) {
            const auto temp_drawable = choice_icons_[choice_idx]->createCopy();
            temp_drawable->replaceColour(juce::Colour(0, 0, 0), base_.getTextColor());
            temp_drawable->drawWithin(g, bound, juce::RectanglePlacement::Flags::centred, 1.0);
        }
    }

    void ClickCombobox::mouseDown(const juce::MouseEvent &) {
        combo_box_.setSelectedItemIndex(
            (combo_box_.getSelectedItemIndex() + 1) % combo_box_.getNumItems(),
            juce::sendNotificationSync);
    }

    void ClickCombobox::setSizeScale(const float width_scale, const float height_scale) {
        width_scale_ = width_scale;
        height_scale_ = height_scale;
    }

    void ClickCombobox::comboBoxChanged(juce::ComboBox *) {
        if (combo_box_.getSelectedItemIndex() >= 0 && !item_labels_.empty()) {
            SettableTooltipClient::setTooltip(item_labels_[static_cast<size_t>(combo_box_.getSelectedItemIndex())]);
        }
        repaint();
    }
} // zlgui
