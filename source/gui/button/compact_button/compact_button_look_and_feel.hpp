// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

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
            const auto isPressed = button.getToggleState() ^ reverse;

            auto bounds = button.getLocalBounds().toFloat();
            if (withShadow) {
                bounds = uiBase.drawShadowEllipse(g, bounds, uiBase.getFontSize() * 0.4f * shrinkScale, {});
                bounds = uiBase.drawInnerShadowEllipse(g, bounds, uiBase.getFontSize() * 0.15f * shrinkScale,
                                                       {.flip = true});
            } else {
                bounds = uiBase.getShadowEllipseArea(bounds, uiBase.getFontSize() * 0.3f * shrinkScale, {});
                g.setColour(uiBase.getBackgroundColor());
                g.fillEllipse(bounds);
            }
            if (isPressed) {
                if (withShadow) {
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
            if (editable) {
                if (drawable == nullptr) {
                    const auto textBound = button.getLocalBounds().toFloat();
                    if (isPressed) {
                        g.setColour(uiBase.getTextColor().withAlpha(1.f));
                    } else {
                        g.setColour(uiBase.getTextColor().withAlpha(0.5f));
                    }
                    g.setFont(uiBase.getFontSize() * scale);
                    g.drawText(button.getButtonText(), textBound.toNearestInt(), juce::Justification::centred);
                } else {
                    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * .5f * scale;
                    const auto drawBound = bounds.withSizeKeepingCentre(radius, radius);
                    if (isPressed) {
                        internalImg->drawWithin(g, drawBound, juce::RectanglePlacement::Flags::centred, 1.f);
                    } else {
                        internalImg->drawWithin(g, drawBound, juce::RectanglePlacement::Flags::centred, .5f);
                    }
                }
            }
        }

        inline void setEditable(const bool f) { editable = f; }

        [[nodiscard]] inline float getDepth() const { return buttonDepth; }

        inline void setDepth(const float x) { buttonDepth = x; }

        inline void setDrawable(juce::Drawable *x) {
            drawable = x;
            updateImages();
        }

        void enableShadow(const bool f) { withShadow = f; }

        void setScale(const float x) { scale = x; }

        void setReverse(const bool f) { reverse = f; }

        void setShrinkScale(const float x) { shrinkScale = x; }

        void updateImages() {
            if (drawable != nullptr) {
                internalImg = drawable->createCopy();
                internalImg->replaceColour(juce::Colour(0, 0, 0), uiBase.getTextColor());
            }
        }

    private:
        bool editable{true}, reverse{false}, withShadow{true};
        float buttonDepth{0.f}, shrinkScale{1.f}, scale{1.f};
        juce::Drawable *drawable = nullptr;
        std::unique_ptr<juce::Drawable> internalImg;

        UIBase &uiBase;
    };
}

#endif //COMPACT_BUTTON_LOOK_AND_FEEL_H
