// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "compact_button.hpp"

namespace zlInterface {
    CompactButton::CompactButton(const juce::String &labelText, UIBase &base, const multilingual::labels labelIdx)
        : uiBase(base), lookAndFeel(uiBase) {
        button.setClickingTogglesState(true);
        button.setButtonText(labelText);
        button.setLookAndFeel(&lookAndFeel);
        button.onClick = [this]() { this->buttonDownAnimation(); };
        addAndMakeVisible(button);

        setEditable(true);
        if (labelIdx != multilingual::labels::labelNum) {
            button.setTooltip(uiBase.getToolTipText(labelIdx));
        }
    }

    CompactButton::~CompactButton() {
        button.setLookAndFeel(nullptr);
    }

    void CompactButton::resized() {
        if (fit.load()) {
            button.setBounds(getLocalBounds());
        } else {
            auto bound = getLocalBounds().toFloat();
            const auto radius = juce::jmin(bound.getHeight(), bound.getWidth());
            bound = bound.withSizeKeepingCentre(radius, radius);
            button.setBounds(bound.toNearestInt());
        }
    }

    void CompactButton::buttonDownAnimation() {
        if (button.getToggleState() && lookAndFeel.getDepth() < 0.1f) {
            lookAndFeel.setDepth(1.f);
            button.repaint();
        } else if (!button.getToggleState()) {
            lookAndFeel.setDepth(0.f);
            button.repaint();
        }
    }
}
