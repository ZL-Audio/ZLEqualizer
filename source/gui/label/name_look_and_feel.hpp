// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../interface_definitions.hpp"

namespace zlgui {
    class NameLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        explicit NameLookAndFeel(UIBase &base) : uiBase(base) {
        }

        void drawLabel(juce::Graphics &g, juce::Label &label) override {
            if (label.isBeingEdited()) {
                return;
            }
            if (editable) {
                g.setColour(uiBase.getTextColor().withMultipliedAlpha(alpha));
            } else {
                g.setColour(uiBase.getTextInactiveColor().withMultipliedAlpha(alpha));
            }
            g.setFont(uiBase.getFontSize() * fontScale);
            g.drawText(label.getText(), label.getLocalBounds().toFloat(), label.getJustificationType());
        }

        inline void setEditable(const bool f) { editable = f; }

        inline void setAlpha(const float x) { alpha = x; }

        inline void setFontScale(const float x) { fontScale = x; }

    private:
        bool editable{true};
        float alpha{1.f};
        float fontScale{FontNormal};

        UIBase &uiBase;
    };
}
