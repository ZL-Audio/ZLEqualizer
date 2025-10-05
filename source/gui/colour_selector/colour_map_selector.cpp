// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "colour_map_selector.hpp"

namespace zlgui::colour_selector {
    ColourMapSelector::ColourMapSelector(zlgui::UIBase& base, const float box_width)
        : base_(base),
          map_box_(zlstate::PColourMapIdx::kChoices, base_),
          map_box_width_p_(box_width) {
        addAndMakeVisible(map_box_);
        map_box_.getBox().addListener(this);
    }

    void ColourMapSelector::paint(juce::Graphics& g) {
        auto bound = getLocalBounds().toFloat();

        bound.removeFromLeft(bound.getWidth() * map_box_width_p_ + base_.getFontSize());
        g.setColour(base_.getTextColor().withAlpha(.875f));
        g.fillRect(bound);
        bound = bound.withSizeKeepingCentre(bound.getWidth() - base_.getFontSize() * .375f,
                                            bound.getHeight() - base_.getFontSize() * .375f);
        const auto& current_colour_map = zlgui::kColourMaps[static_cast<size_t>(map_box_.getBox().getSelectedId() - 1)];
        const auto single_colour_map_width = bound.getWidth() / static_cast<float>(current_colour_map.size());
        for (const auto& colour : current_colour_map) {
            g.setColour(colour);
            g.fillRect(bound.removeFromLeft(single_colour_map_width));
        }
    }

    void ColourMapSelector::resized() {
        auto bound = getLocalBounds().toFloat();
        const auto box_bound = bound.removeFromLeft(bound.getWidth() * map_box_width_p_);
        map_box_.setBounds(box_bound.toNearestInt());
    }

    void ColourMapSelector::comboBoxChanged(juce::ComboBox*) {
        repaint();
    }
} // zlgui
