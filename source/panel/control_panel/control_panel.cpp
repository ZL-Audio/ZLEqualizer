// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "control_panel.hpp"

namespace zlpanel {
    ControlPanel::ControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                               const multilingual::TooltipHelper& tooltip_helper) :
        p_ref_(p), base_(base),
        left_control_panel_(p, base, tooltip_helper),
        right_control_panel_(p, base, tooltip_helper) {
        left_control_panel_.setBufferedToImage(true);
        addAndMakeVisible(left_control_panel_);

        right_control_panel_.setBufferedToImage(true);
        addAndMakeVisible(right_control_panel_);

        setInterceptsMouseClicks(false, true);
    }

    int ControlPanel::getIdealWidth() const {
        return left_control_panel_.getIdealWidth() + right_control_panel_.getIdealWidth();
    }

    int ControlPanel::getIdealHeight() const {
        const auto box_height = juce::roundToInt(base_.getFontSize() * kBoxHeightScale);
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);

        return 3 * box_height + button_height + 5 * padding;
    }

    void ControlPanel::resized() {
        auto bound = getLocalBounds();
        const auto left_width = left_control_panel_.getIdealWidth();
        center_bound = bound.withSizeKeepingCentre(left_width, bound.getHeight());
        left_bound = bound.removeFromLeft(left_width);
        right_control_panel_.setBounds(bound);
        if (dynamic_on_ptr_ != nullptr) {
            turnOnOffDynamic(c_dynamic_on_);
        }
    }

    void ControlPanel::repaintCallBackSlow() {
        if (dynamic_on_ptr_ != nullptr) {
            const auto dynamic_on = dynamic_on_ptr_->load(std::memory_order::relaxed) > .5f;
            if (dynamic_on != c_dynamic_on_) {
                c_dynamic_on_ = dynamic_on;
                turnOnOffDynamic(c_dynamic_on_);
                left_control_panel_.turnOnOffDynamic(c_dynamic_on_);
            }
        }
        left_control_panel_.repaintCallBackSlow();
        right_control_panel_.repaintCallBackSlow();
    }

    void ControlPanel::updateBand() {
        if (base_.getSelectedBand() < zlp::kBandNum) {
            const auto band_s = std::to_string(base_.getSelectedBand());
            dynamic_on_ptr_ = p_ref_.parameters_.getRawParameterValue(zlp::PDynamicON::kID + band_s);
            turnOnOffDynamic(dynamic_on_ptr_->load(std::memory_order::relaxed) > .5f);
            setVisible(true);
            left_control_panel_.updateBand();
            right_control_panel_.updateBand();
        } else {
            dynamic_on_ptr_ = nullptr;
            setVisible(false);
        }
    }

    void ControlPanel::updateSampleRate(const double sample_rate) {
        double freq_max;
        if (sample_rate > 352000.0) {
            freq_max = 160000.0;
        } else if (sample_rate > 176000.0) {
            freq_max = 80000.0;
        } else if (sample_rate > 88000.0) {
            freq_max = 40000.0;
        } else {
            freq_max = 20000.0;
        }
        left_control_panel_.updateFreqMax(freq_max);
        right_control_panel_.updateFreqMax(freq_max);
    }

    void ControlPanel::turnOnOffDynamic(const bool dynamic_on) {
        left_control_panel_.setBounds(dynamic_on ? left_bound : center_bound);
        right_control_panel_.setVisible(dynamic_on);
    }
}
