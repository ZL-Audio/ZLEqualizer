// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "top_panel.hpp"

namespace zlpanel {
    TopPanel::TopPanel(PluginProcessor& p, zlgui::UIBase& base,
                       multilingual::TooltipHelper& tooltip_helper) :
        logo_panel_(p, base, tooltip_helper) {
        logo_panel_.setBufferedToImage(true);
        addAndMakeVisible(logo_panel_);

        setInterceptsMouseClicks(false, true);
    }

    void TopPanel::resized() {
        auto bound = getLocalBounds();
        logo_panel_.setBounds(bound.removeFromLeft(bound.getHeight() * 2));
    }
}
