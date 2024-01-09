// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "grid_panel.hpp"

namespace zlPanel {
    GridPanel::GridPanel(zlInterface::UIBase &base) : uiBase(base) {
        setBufferedToImage(true);
    }

    GridPanel::~GridPanel() = default;

    void GridPanel::paint(juce::Graphics &g) {
        auto bound = getLocalBounds().toFloat();

        const auto thickness = uiBase.getFontSize() * 0.1f;
        g.setFont(uiBase.getFontSize() * zlInterface::FontLarge);
        for (size_t i = 0; i < backgroundFreqs.size(); ++i) {
            const auto x = backgroundFreqs[i] * bound.getWidth() + bound.getX();
            g.setColour(uiBase.getTextLightColor());
            g.drawLine(x, bound.getY(), x, bound.getBottom(), thickness);

            const auto textBound = juce::Rectangle<float>(x - uiBase.getFontSize() * 3 - uiBase.getFontSize() * 0.125f,
                                                          bound.getBottom() - uiBase.getFontSize() * 2,
                                                          uiBase.getFontSize() * 3, uiBase.getFontSize() * 2);
            g.setColour(uiBase.getTextInactiveColor());
            g.drawText(backgroundFreqsNames[i], textBound, juce::Justification::bottomRight);
        }

        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());
        g.setColour(uiBase.getTextLightColor());
        for (auto &d:backgroundDBs) {
            const auto y = d * bound.getHeight() + bound.getY();
            g.drawLine(bound.getX(), y, bound.getRight(), y, thickness);
        }
    }
}
