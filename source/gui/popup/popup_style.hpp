// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_graphics/juce_graphics.h>

namespace zlgui::popup {
    static constexpr auto kTextScale = 1.5f;
    static constexpr auto kBackgroundAlpha = .9f;
    static constexpr auto kSurfaceTint = .05f;
    static constexpr auto kPaddingScale = .5f;

    inline float textFontSize(const float base_font_size) {
        return kTextScale * base_font_size;
    }

    inline int paddingSize(const float base_font_size) {
        return juce::roundToInt(base_font_size * kPaddingScale);
    }

    inline juce::Colour surfaceColour(const juce::Colour background, const juce::Colour text) {
        return background.interpolatedWith(text, kSurfaceTint);
    }

    inline void paintSelectableCard(juce::Graphics& g, const juce::Rectangle<int> bounds,
                                    const juce::Colour text_colour, const float font_size,
                                    const bool selected, const bool hovered) {
        if (!selected && !hovered) {
            return;
        }
        g.setColour(text_colour.withAlpha(selected ? .105f : .045f));
        g.fillRoundedRectangle(bounds.toFloat().reduced(font_size * .16f), font_size * .35f);
    }
}
