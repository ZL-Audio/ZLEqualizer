// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "credit_panel.hpp"

namespace zlpanel {
    CreditPanel::CreditPanel(zlgui::UIBase& base) :
        base_(base) {
    }

    void CreditPanel::paint(juce::Graphics& g) {
        g.setColour(base_.getTextColour());
        const auto padding = std::round(base_.getFontSize());
        const auto bound = getLocalBounds().toFloat().reduced(2 * padding, padding);
        const auto tl = getTipTextLayout(kText, bound.getWidth(), bound.getHeight());
        tl.draw(g, bound);
    }

    int CreditPanel::getIdeatlHeight() const {
        return static_cast<int>(std::ceil(base_.getFontSize() * 1.5f * 33));
    }
}
