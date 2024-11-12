// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "colour_map_selector.hpp"

namespace zlInterface {
    ColourMapSelector::ColourMapSelector(zlInterface::UIBase &base, const float boxWidth)
        : uiBase(base),
          mapBox("", zlState::colourMapIdx::choices, uiBase),
          mapBoxWidthP(boxWidth) {
        addAndMakeVisible(mapBox);
        mapBox.getBox().addListener(this);
    }

    void ColourMapSelector::paint(juce::Graphics &g) {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(),
                                            uiBase.getFontSize() * FontLarge * 1.75f);
        bound.removeFromLeft(bound.getWidth() * mapBoxWidthP + uiBase.getFontSize());
        g.setColour(uiBase.getTextColor().withAlpha(.875f));
        g.fillRect(bound);
        bound = bound.withSizeKeepingCentre(bound.getWidth() - uiBase.getFontSize() * .375f,
                                            bound.getHeight() - uiBase.getFontSize() * .375f);
        const auto &currentColourMap = zlInterface::colourMaps[static_cast<size_t>(mapBox.getBox().getSelectedId() - 1)];
        const auto singleColourMapWidth = bound.getWidth() / static_cast<float>(currentColourMap.size());
        for (const auto &colour : currentColourMap) {
            g.setColour(colour);
            g.fillRect(bound.removeFromLeft(singleColourMapWidth));
        }
    }

    void ColourMapSelector::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(),
                                            uiBase.getFontSize() * FontLarge * 1.75f);
        const auto boxBound = bound.removeFromLeft(bound.getWidth() * mapBoxWidthP);
        mapBox.setBounds(boxBound.toNearestInt());
    }

    void ColourMapSelector::comboBoxChanged(juce::ComboBox *comboBoxThatHasChanged) {
        juce::ignoreUnused(comboBoxThatHasChanged);
        repaint();
    }
} // zlInterface
