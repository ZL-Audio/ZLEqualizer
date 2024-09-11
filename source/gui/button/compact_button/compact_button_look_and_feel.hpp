// Copyright (C) 2024 - zsliu98
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
    class CompactButtonLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        explicit CompactButtonLookAndFeel(UIBase &base) : uiBase(base) {
        }

        void drawToggleButton(juce::Graphics &g,
                              juce::ToggleButton &button,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override {
            juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
            const auto isPressed = button.getToggleState() ^ reverse.load();

            auto bounds = button.getLocalBounds().toFloat();
            if (withShadow.load()) {
                bounds = uiBase.drawShadowEllipse(g, bounds, uiBase.getFontSize() * 0.4f * shrinkScale.load(), {});
                bounds = uiBase.drawInnerShadowEllipse(g, bounds, uiBase.getFontSize() * 0.15f * shrinkScale.load(), {.flip = true});
            } else {
                bounds = uiBase.getShadowEllipseArea(bounds, uiBase.getFontSize() * 0.3f * shrinkScale.load(), {});
                g.setColour(uiBase.getBackgroundColor());
                g.fillEllipse(bounds);
            }
            if (isPressed) {
                if (withShadow.load()) {
                    const auto innerBound = uiBase.getShadowEllipseArea(bounds, uiBase.getFontSize() * 0.1f, {});
                    uiBase.drawInnerShadowEllipse(g, innerBound, uiBase.getFontSize() * 0.375f, {
                                                      .darkShadowColor = uiBase.getDarkShadowColor().
                                                      withMultipliedAlpha(buttonDepth),
                                                      .brightShadowColor = uiBase.getBrightShadowColor().
                                                      withMultipliedAlpha(buttonDepth),
                                                      .changeDark = true,
                                                      .changeBright = true
                                                  });
                }
            }
            if (editable.load()) {
                if (drawable == nullptr) {
                    const auto textBound = button.getLocalBounds().toFloat();
                    if (isPressed) {
                        g.setColour(uiBase.getTextColor().withAlpha(1.f));
                    } else {
                        g.setColour(uiBase.getTextColor().withAlpha(0.5f));
                    }
                    g.setFont(uiBase.getFontSize() * fontScale.load());
                    g.drawText(button.getButtonText(), textBound.toNearestInt(), juce::Justification::centred);
                } else {
                    const auto tempDrawable = drawable->createCopy();
                    tempDrawable->replaceColour(juce::Colour(0, 0, 0), uiBase.getTextColor());
                    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * .5f;
                    const auto drawBound = bounds.withSizeKeepingCentre(radius, radius);
                    if (isPressed) {
                        tempDrawable->drawWithin(g, drawBound, juce::RectanglePlacement::Flags::centred, 1.f);
                    } else {
                        tempDrawable->drawWithin(g, drawBound, juce::RectanglePlacement::Flags::centred, .5f);
                    }
                }
            }
        }

        inline void setEditable(const bool f) { editable.store(f); }

        inline float getDepth() const { return buttonDepth.load(); }

        inline void setDepth(const float x) { buttonDepth = x; }

        inline void setDrawable(juce::Drawable *x) {
            drawable = x;
        }

        void enableShadow(const bool f) { withShadow.store(f); }

        void setReverse(const bool f) { reverse.store(f); }

        void setLabelScale(const float x) { fontScale.store(x); }

        void setShrinkScale(const float x) { shrinkScale.store(x); }

    private:
        std::atomic<bool> editable{true}, reverse{false}, withShadow{true};
        std::atomic<float> buttonDepth = 0.f, fontScale{1.f}, shrinkScale{1.f};
        juce::Drawable *drawable = nullptr;

        UIBase &uiBase;
    };
}

#endif //COMPACT_BUTTON_LOOK_AND_FEEL_H
