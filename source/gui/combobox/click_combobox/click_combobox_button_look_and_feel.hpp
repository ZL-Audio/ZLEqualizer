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
    class ClickComboboxButtonLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        explicit ClickComboboxButtonLookAndFeel(UIBase &base, juce::String label)
            : ui_base_(base), label_string_(std::move(label)) {
        }

        void drawDrawableButton(juce::Graphics &g, juce::DrawableButton &button,
                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
            juce::ignoreUnused(button, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

            if (editable_) {
                g.setColour(ui_base_.getTextColor().withMultipliedAlpha(alpha_.load()));
            } else {
                g.setColour(ui_base_.getTextInactiveColor().withMultipliedAlpha(alpha_.load()));
            }
            g.setFont(ui_base_.getFontSize() * font_scale_.load());
            auto bound = button.getLocalBounds().toFloat();
            bound.removeFromTop(u_pad_.load());
            bound.removeFromBottom(d_pad_.load());
            bound.removeFromLeft(l_pad_.load());
            bound.removeFromRight(r_pad_.load());
            g.drawText(label_string_, bound, justification_.load());
        }

        inline void setEditable(const bool f) { editable_.store(f); }

        inline void setAlpha(const float x) { alpha_.store(x); }

        inline void setFontScale(const float x) { font_scale_.store(x); }

        inline void setJustification(const juce::Justification j) { justification_.store(j); }

        inline void setPadding(const float l, const float r, const float u, const float d) {
            l_pad_.store(l);
            r_pad_.store(r);
            u_pad_.store(u);
            d_pad_.store(d);
        }

    private:
        std::atomic<bool> editable_{true};
        std::atomic<float> alpha_{1.f};
        std::atomic<float> font_scale_{kFontNormal};
        std::atomic<juce::Justification> justification_{juce::Justification::centred};
        std::atomic<float> l_pad_{0.f}, r_pad_{0.f}, u_pad_{0.f}, d_pad_{0.f};

        UIBase &ui_base_;
        juce::String label_string_;
    };
}
