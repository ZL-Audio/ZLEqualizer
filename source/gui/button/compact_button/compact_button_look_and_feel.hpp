// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef COMPACT_BUTTON_LOOK_AND_FEEL_H
#define COMPACT_BUTTON_LOOK_AND_FEEL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlInterface {
    class CompactButtonLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit CompactButtonLookAndFeel(UIBase &base) : uiBase(base) {
        }

        void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button, bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override {
            juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

            auto bounds = button.getLocalBounds().toFloat();
            bounds = uiBase.drawShadowEllipse(g, bounds, uiBase.getFontSize() * 0.5f, {});
            bounds = uiBase.drawInnerShadowEllipse(g, bounds, uiBase.getFontSize() * 0.15f, {.flip = true});
            if (button.getToggleState()) {
                bounds = uiBase.getShadowEllipseArea(bounds, uiBase.getFontSize() * 0.1f, {});
                uiBase.drawInnerShadowEllipse(g, bounds, uiBase.getFontSize() * 0.375f, {
                                                  .darkShadowColor = uiBase.getDarkShadowColor().
                                                  withMultipliedAlpha(buttonDepth),
                                                  .brightShadowColor = uiBase.getBrightShadowColor().
                                                  withMultipliedAlpha(buttonDepth),
                                                  .changeDark = true,
                                                  .changeBright = true
                                              });
            }
            if (editable.load()) {
                if (drawable == nullptr) {
                    const auto textBound = button.getLocalBounds().toFloat();
                    if (button.getToggleState()) {
                        g.setColour(uiBase.getTextColor().withAlpha(1.f));
                    } else {
                        g.setColour(uiBase.getTextColor().withAlpha(0.5f));
                    }
                    g.setFont(uiBase.getFontSize() * FontLarge);
                    g.drawText(button.getButtonText(), textBound.toNearestInt(), juce::Justification::centred);
                } else {
                    const auto drawBonud = button.getLocalBounds().toFloat().withSizeKeepingCentre(
                        uiBase.getFontSize() * FontLarge, uiBase.getFontSize() * FontLarge);
                    // g.setColour(uiBase.getTextInactiveColor());
                    // g.fillRect(drawBonud);
                    if (button.getToggleState()) {
                        drawable->drawWithin(g, drawBonud, juce::RectanglePlacement::Flags::centred, 1.f);
                    } else {
                        drawable->drawWithin(g, drawBonud, juce::RectanglePlacement::Flags::centred, .5f);
                    }
                }
            }
        }

        inline void setEditable(const bool f) { editable.store(f); }

        inline float getDepth() const { return buttonDepth.load(); }

        inline void setDepth(const float x) { buttonDepth = x; }

        inline void setDrawable(juce::Drawable *x) {
            drawable = x;
            drawable->replaceColour(juce::Colour(0, 0, 0), uiBase.getTextColor());
        }

    private:
        std::atomic<bool> editable = true;
        std::atomic<float> buttonDepth = 0.f;
        juce::Drawable *drawable = nullptr;

        UIBase &uiBase;
    };
}

#endif //COMPACT_BUTTON_LOOK_AND_FEEL_H
