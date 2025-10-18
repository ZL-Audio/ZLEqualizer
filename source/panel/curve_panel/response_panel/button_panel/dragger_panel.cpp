// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "dragger_panel.hpp"

namespace zlpanel {
    DraggerPanel::DraggerPanel(PluginProcessor& p,
                               zlgui::UIBase& base,
                               const multilingual::TooltipHelper&) :
        p_ref_(p), base_(base),
        draggers_(make_dragger_array(base, std::make_index_sequence<zlp::kBandNum>())),
        target_draggers_(make_dragger_array(base, std::make_index_sequence<zlp::kBandNum>())),
        side_dragger_(base) {
        juce::ignoreUnused(p_ref_);
        side_dragger_.getLAF().setDraggerShape(zlgui::dragger::DraggerLookAndFeel::kRectangle);
        addChildComponent(side_dragger_);

        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            target_draggers_[band].getButton().setBufferedToImage(true);
            target_draggers_[band].getLAF().setColour(base_.getColourMap1(band));
            target_draggers_[band].getLAF().setDraggerShape(zlgui::dragger::DraggerLookAndFeel::kUpDownArrow);
            addChildComponent(target_draggers_[band]);
        }

        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            draggers_[band].getButton().setBufferedToImage(true);
            draggers_[band].getLAF().setColour(base_.getColourMap1(band));
            draggers_[band].getLAF().setDraggerShape(zlgui::dragger::DraggerLookAndFeel::kRound);
            addChildComponent(draggers_[band]);
        }

        lookAndFeelChanged();
        setInterceptsMouseClicks(false, true);
    }

    void DraggerPanel::resized() {

    }

    void DraggerPanel::updateBand() {

    }

    void DraggerPanel::updateDrawingParas(const size_t band, const zlp::FilterStatus filter_status,
                                          const bool is_dynamic_on, const bool is_same_stereo) {
        const auto is_not_off_and_dynamic_on = (filter_status != zlp::FilterStatus::kOff) && is_dynamic_on;
        const auto is_selected = base_.getSelectedBand() == band;
        draggers_[band].setVisible(filter_status != zlp::FilterStatus::kOff);
        target_draggers_[band].setVisible(is_not_off_and_dynamic_on && is_selected);
        side_dragger_.setVisible(is_not_off_and_dynamic_on && is_selected);
        is_not_off_and_dynamic_on_[band] = is_not_off_and_dynamic_on;
        float alpha = 1.f;
        if (filter_status == zlp::FilterStatus::kBypass) {
            alpha *= kBypassAlphaMultiplier;
        }
        if (!is_same_stereo) {
            alpha *= kDiffStereoAlphaMultiplier;
        }
        draggers_[band].getButton().setAlpha(alpha);
        target_draggers_[band].getButton().setAlpha(alpha);
    }

    void DraggerPanel::lookAndFeelChanged() {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            draggers_[band].getLAF().setColour(base_.getColourMap1(band));
        }
    }
}
