// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef ZL_COMPACT_LINEAR_SLIDER_LOOK_AND_FEEL_H
#define ZL_COMPACT_LINEAR_SLIDER_LOOK_AND_FEEL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlInterface {
    class CompactLinearSliderLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit CompactLinearSliderLookAndFeel(UIBase &base): uiBase(base) {
        }

        void drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              const juce::Slider::SliderStyle, juce::Slider &slider) override {
            juce::ignoreUnused(slider, minSliderPos, maxSliderPos);
            auto bound = juce::Rectangle<int>(x, y, width, height).toFloat();
            uiBase.fillRoundedInnerShadowRectangle(g, bound, uiBase.getFontSize() * 0.5f, {.blurRadius = 0.66f});

            juce::Path mask;
            mask.addRoundedRectangle(bound, uiBase.getFontSize() * 0.5f);
            g.saveState();
            g.reduceClipRegion(mask);
            auto proportion = sliderPos / static_cast<float>(width);
            auto shadowBound = bound.withWidth(proportion * bound.getWidth());
            g.setColour(uiBase.getTextHideColor());
            g.fillRect(shadowBound);
            g.restoreState();
        }

        juce::Slider::SliderLayout getSliderLayout(juce::Slider &slider) override {
            juce::Slider::SliderLayout layout;
            layout.sliderBounds = slider.getLocalBounds();
            return layout;
        }

        inline void setEditable(const bool f) { editable.store(f); }

        inline void setTextAlpha(const float x) { textAlpha.store(x); }

    private:
        std::atomic<bool> editable = true;
        std::atomic<float> textAlpha;

        UIBase &uiBase;
    };
}

#endif //ZL_COMPACT_LINEAR_SLIDER_LOOK_AND_FEEL_H
