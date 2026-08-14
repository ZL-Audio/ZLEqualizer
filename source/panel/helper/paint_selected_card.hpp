// Copyright (C) 2026 - zsliu98
// This file is part of ZLSpectrumEqualizer
//
// ZLSpectrumEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSpectrumEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSpectrumEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_graphics/juce_graphics.h>

namespace zlpanel {
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
