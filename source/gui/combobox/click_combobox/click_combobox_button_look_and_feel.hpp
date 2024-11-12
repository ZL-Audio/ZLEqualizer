// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_CLICK_COMBOBOX_BUTTON_LOOK_AND_FEEL_HPP
#define ZLEqualizer_CLICK_COMBOBOX_BUTTON_LOOK_AND_FEEL_HPP

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../interface_definitions.hpp"

namespace zlInterface {
    class ClickComboboxButtonLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        explicit ClickComboboxButtonLookAndFeel(UIBase &base, juce::String label)
            : uiBase(base), labelString(std::move(label)) {
        }

        void drawDrawableButton(juce::Graphics &g, juce::DrawableButton &button,
                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
            juce::ignoreUnused(button, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

            if (editable) {
                g.setColour(uiBase.getTextColor().withMultipliedAlpha(alpha.load()));
            } else {
                g.setColour(uiBase.getTextInactiveColor().withMultipliedAlpha(alpha.load()));
            }
            g.setFont(uiBase.getFontSize() * fontScale.load());
            auto bound = button.getLocalBounds().toFloat();
            bound.removeFromTop(uPadding.load());
            bound.removeFromBottom(dPadding.load());
            bound.removeFromLeft(lPadding.load());
            bound.removeFromRight(rPadding.load());
            g.drawText(labelString, bound, justification.load());
        }

        inline void setEditable(const bool f) { editable.store(f); }

        inline void setAlpha(const float x) { alpha.store(x); }

        inline void setFontScale(const float x) { fontScale.store(x); }

        inline void setJustification(const juce::Justification j) { justification.store(j); }

        inline void setPadding(const float l, const float r, const float u, const float d) {
            lPadding.store(l);
            rPadding.store(r);
            uPadding.store(u);
            dPadding.store(d);
        }

    private:
        std::atomic<bool> editable{true};
        std::atomic<float> alpha{1.f};
        std::atomic<float> fontScale{FontNormal};
        std::atomic<juce::Justification> justification{juce::Justification::centred};
        std::atomic<float> lPadding{0.f}, rPadding{0.f}, uPadding{0.f}, dPadding{0.f};

        UIBase &uiBase;
        juce::String labelString;
    };
}

#endif //ZLEqualizer_CLICK_COMBOBOX_BUTTON_LOOK_AND_FEEL_HPP
