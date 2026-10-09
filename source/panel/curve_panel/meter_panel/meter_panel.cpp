// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "meter_panel.hpp"

namespace zlpanel {
    MeterPanel::MeterPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base),
        meter_background_panel_(p, base),
        meter_display_panel_(p, base) {

        meter_background_panel_.setBufferedToImage(true);
        addAndMakeVisible(meter_background_panel_);

        addAndMakeVisible(meter_display_panel_);
    }

    MeterPanel::~MeterPanel() = default;

    int MeterPanel::getIdealWidth() const {
        return juce::roundToInt(base_.getFontSize() * 3.f);
    }

    void MeterPanel::resized() {
        meter_background_panel_.setBounds(getLocalBounds());
        if (meter_display_panel_.getBounds() == getLocalBounds()) {
            meter_display_panel_.resized();
        } else {
            meter_display_panel_.setBounds(getLocalBounds());
        }
    }

    void MeterPanel::repaintCallBackSlow() {
        meter_display_panel_.repaintCallBackSlow();
    }
}
