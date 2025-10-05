// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "compact_button.hpp"

namespace zlgui::button {
    CompactButton::CompactButton(const juce::String& labelText, UIBase& base, const juce::String& tooltip_text)
        : base_(base), laf_(base_) {
        button_.setToggleable(true);
        button_.setClickingTogglesState(true);
        button_.setButtonText(labelText);
        button_.setLookAndFeel(&laf_);
        button_.onClick = [this]() { this->buttonDownAnimation(); };
        addAndMakeVisible(button_);

        setEditable(true);
        if (tooltip_text.length() > 0) {
            button_.setTooltip(tooltip_text);
        }
    }

    CompactButton::~CompactButton() {
        button_.setLookAndFeel(nullptr);
    }

    void CompactButton::resized() {
        auto bound = getLocalBounds();
        const auto radius = juce::jmin(bound.getHeight(), bound.getWidth());
        bound = bound.withSizeKeepingCentre(radius, radius);
        button_.setBounds(bound);
    }

    void CompactButton::buttonDownAnimation() {
        if (button_.getToggleState() && laf_.getDepth() < 0.1f) {
            laf_.setDepth(1.f);
            button_.repaint();
        } else if (!button_.getToggleState()) {
            laf_.setDepth(0.f);
            button_.repaint();
        }
    }
}
