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

namespace zlgui::label {
    class NameLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        explicit NameLookAndFeel(UIBase &base) : base_(base) {
        }

        void drawLabel(juce::Graphics &g, juce::Label &label) override {
            if (label.isBeingEdited()) {
                return;
            }
            g.setColour(base_.getTextColor().withMultipliedAlpha(alpha_));
            g.setFont(base_.getFontSize() * font_scale_);
            g.drawText(label.getText(), label.getLocalBounds().toFloat(), label.getJustificationType());
        }

        inline void setAlpha(const float x) { alpha_ = x; }

        inline void setFontScale(const float x) { font_scale_ = x; }

    private:
        UIBase &base_;

        float alpha_{1.f};
        float font_scale_{kFontNormal};
    };
}
