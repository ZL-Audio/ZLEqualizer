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
        base_(base), updater_(),
        logo_panel_(p, base, tooltip_helper),
        fstruct_box_(zlp::PFilterStructure::kChoices, base,
                     tooltip_helper.getToolTipText(multilingual::kFilterStructure),
                     {tooltip_helper.getToolTipText(multilingual::kMinimumPhase),
                      tooltip_helper.getToolTipText(multilingual::kStateVariable),
                      tooltip_helper.getToolTipText(multilingual::kParallelPhase),
                      tooltip_helper.getToolTipText(multilingual::kMatchedPhase),
                      tooltip_helper.getToolTipText(multilingual::kMixedPhase),
                      tooltip_helper.getToolTipText(multilingual::kLinearPhase)}),
        fstruct_attach_(fstruct_box_.getBox(), p.parameters_, zlp::PFilterStructure::kID, updater_) {
        logo_panel_.setBufferedToImage(true);
        addAndMakeVisible(logo_panel_);

        fstruct_box_.setBufferedToImage(true);
        addAndMakeVisible(fstruct_box_);

        setInterceptsMouseClicks(false, true);
    }

    void TopPanel::resized() {
        const auto slider_width = getSliderWidth(base_.getFontSize());
        auto bound = getLocalBounds();
        logo_panel_.setBounds(bound.removeFromLeft(bound.getHeight() * 2));
        fstruct_box_.setBounds(bound.removeFromLeft(slider_width * 2));
    }

    void TopPanel::repaintCallbackSlow() {
        updater_.updateComponents();
    }
}
