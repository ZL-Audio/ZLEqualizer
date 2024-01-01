// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef ZL_LINEAR_SLIDER_LOOK_AND_FEEL_H
#define ZL_LINEAR_SLIDER_LOOK_AND_FEEL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlInterface {
    class LinearSliderLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit LinearSliderLookAndFeel(UIBase &base) {
            uiBase = &base;
        }

        void drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              const juce::Slider::SliderStyle, juce::Slider &slider) override {
            juce::ignoreUnused(slider, minSliderPos, maxSliderPos);
            auto bound = juce::Rectangle<int>(x, y, width, height).toFloat();
            bound = uiBase->getRoundedShadowRectangleArea(bound, uiBase->getFontSize() * 0.5f, {});
            uiBase->fillRoundedInnerShadowRectangle(g, bound, uiBase->getFontSize() * 0.5f, {.blurRadius = 0.66f});

            juce::Path mask;
            mask.addRoundedRectangle(bound, uiBase->getFontSize() * 0.5f);
            g.saveState();
            g.reduceClipRegion(mask);
            auto proportion = sliderPos / static_cast<float>(width);
            auto shadowBound = bound.withWidth(proportion * bound.getWidth());
            g.setColour(uiBase->getTextHideColor());
            g.fillRect(shadowBound);
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
            auto textBounds = localBounds;
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
                g.setFont(uiBase->getFontSize() * FontLarge);
            } else {
                g.setFont(labelArea.getHeight() * 0.6f);
            }
            g.drawSingleLineText(juce::String(label.getText()),
                                 juce::roundToInt(center.x + g.getCurrentFont().getHorizontalScale()),
                                 juce::roundToInt(center.y + g.getCurrentFont().getDescent()),
                                 juce::Justification::horizontallyCentred);
        }

        void setEditable(bool f) {
            editable.store(f);
        }

        void setMouseOver(bool f) {
            mouseOver.store(f);
        }

    private:
        std::atomic<bool> editable = true, mouseOver = false;

        UIBase *uiBase;
    };
}

#endif //ZL_LINEAR_SLIDER_LOOK_AND_FEEL_H
