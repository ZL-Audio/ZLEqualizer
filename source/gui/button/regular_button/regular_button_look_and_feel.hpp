// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef ZL_REGULAR_BUTTON_LOOK_AND_FEEL_H
#define ZL_REGULAR_BUTTON_LOOK_AND_FEEL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlInterface {
    class RegularButtonLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit RegularButtonLookAndFeel(UIBase &base) {
            uiBase = &base;
        }

        void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button, bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override {
            juce::ignoreUnused(shouldDrawButtonAsDown);
            auto bounds = button.getLocalBounds().toFloat();
            // draw button
            bounds = uiBase->fillRoundedShadowRectangle(g, bounds, uiBase->getFontSize() * 0.5f, {});
            uiBase->fillRoundedInnerShadowRectangle(g, bounds, uiBase->getFontSize() * 0.5f, {.blurRadius=0.45f, .flip=true});
            // draw ON/OFF
            if (editable.load()) {
                g.setColour(uiBase->getTextColor());
            } else {
                g.setColour(uiBase->getTextInactiveColor());
            }
            if (uiBase->getFontSize() > 0) {
                g.setFont(uiBase->getFontSize() * FontLarge);
            } else {
                g.setFont(bounds.getHeight() * 0.35f);
            }
            if (button.getToggleState()) {
                g.drawSingleLineText(juce::String("ON"),
                                     juce::roundToInt(bounds.getCentre().x + bounds.getWidth() * 0.22 +
                                                      g.getCurrentFont().getHorizontalScale() * 0.5f),
                                     juce::roundToInt(bounds.getCentre().y + g.getCurrentFont().getDescent()),
                                     juce::Justification::horizontallyCentred);
            } else {
                g.drawSingleLineText(juce::String("OFF"),
                                     juce::roundToInt(bounds.getCentre().x - bounds.getWidth() * 0.22 +
                                                      g.getCurrentFont().getHorizontalScale() * 0.5f),
                                     juce::roundToInt(bounds.getCentre().y + g.getCurrentFont().getDescent()),
                                     juce::Justification::horizontallyCentred);
            }
            g.setColour(TextHideColor);
            if (shouldDrawButtonAsHighlighted) {
                if (!button.getToggleState()) {
                    g.drawSingleLineText(juce::String("ON"),
                                         juce::roundToInt(bounds.getCentre().x + bounds.getWidth() * 0.22 +
                                                          g.getCurrentFont().getHorizontalScale() * 0.5f),
                                         juce::roundToInt(bounds.getCentre().y + g.getCurrentFont().getDescent()),
                                         juce::Justification::horizontallyCentred);
                } else {
                    g.drawSingleLineText(juce::String("OFF"),
                                         juce::roundToInt(bounds.getCentre().x - bounds.getWidth() * 0.22 +
                                                          g.getCurrentFont().getHorizontalScale() * 0.5f),
                                         juce::roundToInt(bounds.getCentre().y + g.getCurrentFont().getDescent()),
                                         juce::Justification::horizontallyCentred);
                }
            }
        }
        inline void setEditable(const bool f) { editable.store(f); }

    private:
        std::atomic<bool> editable = true;

        UIBase *uiBase;
    };
}
#endif //ZL_REGULAR_BUTTON_LOOK_AND_FEEL_H
