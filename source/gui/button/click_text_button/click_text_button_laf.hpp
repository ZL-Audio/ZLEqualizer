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

#include "../../interface_definitions.hpp"

namespace zlgui {
    class ClickTextButtonLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        explicit ClickTextButtonLookAndFeel(UIBase &base) : base_(base) {
        }

        void drawButtonBackground(juce::Graphics &g, juce::Button &,
                                  const juce::Colour &, bool, bool) override {
            g.fillAll(base_.getBackgroundColor());
        }

        void drawButtonText(juce::Graphics &g, juce::TextButton &button,
                            const bool highlight, const bool down) override {
            if (highlight || down) {
                g.setColour(base_.getTextColor());
            } else {
                g.setColour(base_.getTextInactiveColor());
            }
            g.setFont(base_.getFontSize() * font_scale_);
            g.drawText(button.getButtonText(), button.getBounds(), justification_);
        }

        void setJustification(const juce::Justification justification) {
            justification_ = justification;
        }

        void setFontScale(const float font_scale) {
            font_scale_ = font_scale;
        }

    private:
        UIBase &base_;

        float font_scale_{1.f};
        juce::Justification justification_{juce::Justification::centredLeft};
    };
}
