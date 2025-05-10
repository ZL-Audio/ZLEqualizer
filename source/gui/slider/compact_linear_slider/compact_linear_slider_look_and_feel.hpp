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
    class CompactLinearSliderLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit CompactLinearSliderLookAndFeel(UIBase &base): ui_base_(base) {
        }

        void drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height,
                              float slider_pos, float min_slider_pos, float max_slider_pos,
                              const juce::Slider::SliderStyle, juce::Slider &slider) override {
            juce::ignoreUnused(slider, min_slider_pos, max_slider_pos);
            auto bound = juce::Rectangle<int>(x, y, width, height).toFloat();
            ui_base_.fillRoundedInnerShadowRectangle(g, bound, ui_base_.getFontSize() * 0.5f, {.blur_radius = 0.66f});

            juce::Path mask;
            mask.addRoundedRectangle(bound, ui_base_.getFontSize() * 0.5f);
            g.saveState();
            g.reduceClipRegion(mask);
            auto proportion = slider_pos / static_cast<float>(width);
            auto shadow_bound = bound.withWidth(proportion * bound.getWidth());
            g.setColour(ui_base_.getTextHideColor());
            g.fillRect(shadow_bound);
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

        UIBase &ui_base_;
    };
}
