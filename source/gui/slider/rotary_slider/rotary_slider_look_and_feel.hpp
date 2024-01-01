// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef ZL_ROTARY_SLIDER_LOOK_AND_FEEL_H
#define ZL_ROTARY_SLIDER_LOOK_AND_FEEL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlInterface {
    class RotarySliderLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit RotarySliderLookAndFeel(UIBase &base) {
            uiBase = &base;
        }

        void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
                              const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider &slider) override {
            juce::ignoreUnused(slider);
            // calculate values
            auto rotationAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
            auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
            bounds = bounds.withSizeKeepingCentre(diameter, diameter);
            // draw knob
            auto oldBounds = uiBase->drawInnerShadowEllipse(g, bounds, uiBase->getFontSize() * 0.5f, {});
            auto newBounds = uiBase->drawShadowEllipse(g, oldBounds, uiBase->getFontSize() * 0.5f, {});
            uiBase->drawInnerShadowEllipse(g, newBounds, uiBase->getFontSize() * 0.15f, {.flip = true});
            // draw arrow
            auto arrowUnit = (diameter - newBounds.getWidth()) * 0.5f;
            auto arrowBound = juce::Rectangle<float>(
                -0.5f * arrowUnit + bounds.getCentreX() +
                (0.5f * diameter - 0.5f * arrowUnit) * std::sin(rotationAngle),
                -0.5f * arrowUnit + bounds.getCentreY() +
                (0.5f * diameter - 0.5f * arrowUnit) * (-std::cos(rotationAngle)),
                arrowUnit, arrowUnit);
            auto arrowStartBound = juce::Rectangle<float>(
                -0.5f * arrowUnit + bounds.getCentreX() +
                (0.5f * diameter - 0.5f * arrowUnit) * std::sin(rotaryStartAngle),
                -0.5f * arrowUnit + bounds.getCentreY() +
                (0.5f * diameter - 0.5f * arrowUnit) * (-std::cos(rotaryStartAngle)),
                arrowUnit, arrowUnit);
            juce::Path mask;
            mask.addEllipse(bounds);
            mask.setUsingNonZeroWinding(false);
            mask.addEllipse(newBounds);
            g.saveState();
            g.reduceClipRegion(mask);
            uiBase->drawShadowEllipse(g, arrowBound, uiBase->getFontSize() * 0.5f,
                                      {.fit = false, .drawBright = false, .drawDark = true});
            uiBase->drawShadowEllipse(g, arrowStartBound, uiBase->getFontSize() * 0.5f,
                                      {
                                          .fit = false, .drawBright = false, .drawDark = true,
                                          .mainColour = uiBase->getBackgroundColor().withAlpha(
                                              uiBase->getTextColor().getAlpha()),
                                          .changeMain = true
                                      });

            juce::Path filling;
            filling.addPieSegment(bounds, rotaryStartAngle, rotationAngle, 0);
            filling.setUsingNonZeroWinding(false);
            filling.addPieSegment(arrowStartBound, rotaryStartAngle, rotaryStartAngle + juce::MathConstants<float>::pi,
                                  0);
            g.setColour(uiBase->getTextHideColor());
            g.fillPath(filling);
            uiBase->drawInnerShadowEllipse(g, arrowBound, uiBase->getFontSize() * 0.15f, {.flip = true});
            g.restoreState();
        }

        juce::Label *createSliderTextBox(juce::Slider &) override {
            auto *l = new juce::Label();
            l->setJustificationType(juce::Justification::centred);
            l->setInterceptsMouseClicks(false, false);
            return l;
        }

        juce::Slider::SliderLayout getSliderLayout(juce::Slider &slider) override {
            auto localBounds = slider.getLocalBounds().toFloat();
            juce::Slider::SliderLayout layout;
            auto textBounds = localBounds.withSizeKeepingCentre(localBounds.getWidth() * 0.7f,
                                                                localBounds.getHeight() * 0.7f);
            layout.textBoxBounds = textBounds.toNearestInt();
            layout.sliderBounds = slider.getLocalBounds();
            return layout;
        }

        void drawLabel(juce::Graphics &g, juce::Label &label) override {
            if (editable.load()) {
                g.setColour(uiBase->getTextColor());
            } else {
                g.setColour(uiBase->getTextInactiveColor());
            }
            auto labelArea{label.getLocalBounds().toFloat()};
            auto center = labelArea.getCentre();
            if (uiBase->getFontSize() > 0) {
                g.setFont(uiBase->getFontSize() * FontHuge);
            } else {
                g.setFont(labelArea.getHeight() * 0.6f);
            }
            float value = label.getText().getFloatValue();
            juce::String labelToDisplay = juce::String(label.getText()).substring(0, 4);
            if (value < 10000 && labelToDisplay.contains(".")) {
                labelToDisplay = juce::String(label.getText()).substring(0, 5);
            }
            if (value > 10000) {
                value = value / 1000;
                labelToDisplay = juce::String(value).substring(0, 4) + "K";
            }
            g.drawSingleLineText(labelToDisplay,
                                 juce::roundToInt(center.x + g.getCurrentFont().getHorizontalScale()),
                                 juce::roundToInt(center.y + g.getCurrentFont().getDescent()),
                                 juce::Justification::horizontallyCentred);
        }

        void setEditable(bool f) {
            editable.store(f);
        }

    private:
        std::atomic<bool> editable = true;

        UIBase *uiBase;
    };
}
#endif //ZL_ROTARY_SLIDER_LOOK_AND_FEEL_H
