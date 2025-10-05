// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "control_background.hpp"

namespace zlpanel {
    ControlBackground::ControlBackground(zlgui::UIBase& base) :
        base_(base) {
        setInterceptsMouseClicks(false, false);
    }

    void ControlBackground::paint(juce::Graphics& g) {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto bound = getLocalBounds().reduced(padding / 2);
        juce::Path path;
        path.addRoundedRectangle(bound.toFloat(), static_cast<float>(padding) * .5f);

        g.setColour(base_.getBackgroundColor());
        g.fillPath(path);
        const juce::DropShadow shadow{base_.getBackgroundColor(), padding / 2, {0, 0}};
        shadow.drawForPath(g, path);
    }
}
