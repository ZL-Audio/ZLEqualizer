// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "curve_panel.hpp"

namespace zlpanel {
    CurvePanel::CurvePanel(PluginProcessor& p,
                           zlgui::UIBase& base,
                           const multilingual::TooltipHelper& tooltip_helper) :
        scale_panel_(p, base, tooltip_helper) {
        scale_panel_.setBufferedToImage(true);
        addAndMakeVisible(scale_panel_);
    }

    void CurvePanel::resized() {
        auto bound = getLocalBounds();
        scale_panel_.setBounds(bound.removeFromRight(scale_panel_.getIdealWidth()));
    }

    void CurvePanel::repaintCallBack() {

    }

    void CurvePanel::repaintCallBackSlow() {
        scale_panel_.repaintCallBackSlow();
    }

    void CurvePanel::updateBand() {

    }

    void CurvePanel::updateFreqMax(double freq_max) {
        juce::ignoreUnused(freq_max);
    }
}
