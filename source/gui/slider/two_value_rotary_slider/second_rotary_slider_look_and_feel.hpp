// ==============================================================================
// Copyright (C) 2024 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef TWO_VALUE_ROTARY_SLIDER_LOOK_AND_FEEL_H
#define TWO_VALUE_ROTARY_SLIDER_LOOK_AND_FEEL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlInterface {
    class SecondRotarySliderLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit SecondRotarySliderLookAndFeel(UIBase &base) {
            uiBase = &base;
        }

        void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
                              const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider &slider) override {
            juce::ignoreUnused(slider);
            if (!editable.load()) {
                return;
            }
            // calculate values
            auto rotationAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
            auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
            bounds = bounds.withSizeKeepingCentre(diameter, diameter);
            // draw knob
            auto oldBounds = uiBase->getInnerShadowEllipseArea(bounds, uiBase->getFontSize() * 0.5f, {});
            auto newBounds = uiBase->getShadowEllipseArea(oldBounds, uiBase->getFontSize() * 0.5f, {});
            // uiBase->drawInnerShadowEllipse(g, newBounds, uiBase->getFontSize() * 0.15f, {.flip=true});
            // draw arrow
            auto arrowUnit = (diameter - newBounds.getWidth()) * 0.5f;
            auto arrowBound = juce::Rectangle<float>(
                -0.5f * arrowUnit + bounds.getCentreX() +
                (0.5f * diameter - 0.5f * arrowUnit) * std::sin(rotationAngle),
                -0.5f * arrowUnit + bounds.getCentreY() +
                (0.5f * diameter - 0.5f * arrowUnit) * (-std::cos(rotationAngle)),
                arrowUnit, arrowUnit);

            juce::Path mask;
            mask.addEllipse(bounds);
            mask.setUsingNonZeroWinding(false);
            mask.addEllipse(newBounds);
            g.saveState();
            g.reduceClipRegion(mask);
            uiBase->drawShadowEllipse(g, arrowBound, uiBase->getFontSize() * 0.5f,
                                      {
                                          .fit = false, .drawBright = false, .drawDark = true,
                                          .mainColour = uiBase->getColorMap2(0),
                                          .changeMain = true
                                      });
            g.restoreState();
        }

        void setEditable(const bool f) { editable.store(f); }

    private:
        std::atomic<bool> editable = true;

        UIBase *uiBase;
    };
}

#endif //TWO_VALUE_ROTARY_SLIDER_LOOK_AND_FEEL_H
