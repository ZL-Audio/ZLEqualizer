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
        right_control_panel_(p, base, tooltip_helper),
        side_loudness_display_panel_(p, base),
        match_control_panel_(p, base, tooltip_helper) {
        addAndMakeVisible(mouse_event_eater_);

        left_control_panel_.setBufferedToImage(true);
        addAndMakeVisible(left_control_panel_);

        right_control_panel_.setBufferedToImage(true);
        addChildComponent(right_control_panel_);

        addChildComponent(side_loudness_display_panel_);

        match_control_panel_.setBufferedToImage(true);
        addChildComponent(match_control_panel_);

        setInterceptsMouseClicks(false, true);

        base_.getPanelValueTree().addListener(this);
    }

    ControlPanel::~ControlPanel() {
        base_.getPanelValueTree().removeListener(this);
    }

    int ControlPanel::getIdealWidth() const {
        return left_control_panel_.getIdealWidth() + right_control_panel_.getIdealWidth();
    }

    int ControlPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        const auto box_height = getBoxHeight(font_size);
        const auto button_height = getButtonSize(font_size);
        const auto padding = getPaddingSize(font_size);

        return 3 * box_height + button_height + 5 * padding;
    }

    void ControlPanel::resized() {
        const auto font_size = base_.getFontSize();
        auto bound = getLocalBounds();
        const auto padding = getPaddingSize(font_size);

        match_control_panel_.setBounds(bound.withSizeKeepingCentre(
            match_control_panel_.getIdealWidth(), match_control_panel_.getIdealHeight()));

        const auto left_width = left_control_panel_.getIdealWidth();
        center_bound_ = bound.withSizeKeepingCentre(left_width, bound.getHeight());
        mouse_center_bound_ = center_bound_.reduced(padding);
        mouse_full_bound_ = bound.reduced(padding);
        left_bound_ = bound.removeFromLeft(left_width);
        right_control_panel_.setBounds(bound);

        const auto button_height = getButtonSize(font_size);
        const auto slider_height = getSliderHeight(font_size);
        const auto slider_width = getSliderWidth(font_size);
        bound.reduce(padding + padding / 2, padding);
        bound.removeFromTop(button_height);
        const auto h_padding = (bound.getHeight() - 2 * slider_height) / 4;
        bound = bound.removeFromLeft(slider_width);
        bound.removeFromBottom(3 * h_padding + slider_height);
        side_loudness_display_panel_.setBounds(bound.removeFromBottom(slider_height / 4));

        if (dynamic_on_ptr_ != nullptr) {
            changeLeftRightBound(c_dynamic_on_);
        }
    }

    void ControlPanel::repaintCallBack() {
        side_loudness_display_panel_.repaintCallBack();
    }

    void ControlPanel::repaintCallBackSlow() {
        if (dynamic_on_ptr_ != nullptr) {
            const auto dynamic_on = dynamic_on_ptr_->load(std::memory_order::relaxed) > .5f;
            if (dynamic_on != c_dynamic_on_) {
                c_dynamic_on_ = dynamic_on;
                changeLeftRightBound(c_dynamic_on_);
                left_control_panel_.turnOnOffDynamic(c_dynamic_on_);
            }
            left_control_panel_.repaintCallBackSlow();
            right_control_panel_.repaintCallBackSlow();
            side_loudness_display_panel_.repaintCallBackSlow();
        }
    }

    void ControlPanel::updateBand() {
        if (base_.getSelectedBand() < zlp::kBandNum) {
            const auto band_s = std::to_string(base_.getSelectedBand());
            dynamic_on_ptr_ = p_ref_.parameters_.getRawParameterValue(zlp::PDynamicON::kID + band_s);
            changeLeftRightBound(dynamic_on_ptr_->load(std::memory_order::relaxed) > .5f);
            left_control_panel_.updateBand();
            right_control_panel_.updateBand();
            side_loudness_display_panel_.updateBand();
            left_control_panel_.setVisible(true);
        } else {
            dynamic_on_ptr_ = nullptr;
            left_control_panel_.setVisible(false);
            right_control_panel_.setVisible(false);
            side_loudness_display_panel_.setVisible(false);
        }
        base_.setPanelProperty(zlgui::PanelSettingIdx::kDynamicExtraPanel, 0.0);
        repaintCallBackSlow();
    }

    void ControlPanel::updateSampleRate(const double sample_rate) {
        const auto freq_max = freq_helper::getSliderMax(sample_rate);
        left_control_panel_.updateFreqMax(freq_max);
        right_control_panel_.updateFreqMax(freq_max);
    }

    void ControlPanel::changeLeftRightBound(const bool dynamic_on) {
        mouse_event_eater_.setBounds(dynamic_on ? mouse_full_bound_ : mouse_center_bound_);
        left_control_panel_.setBounds(dynamic_on ? left_bound_ : center_bound_);
        right_control_panel_.setVisible(dynamic_on);
        if (!dynamic_on) {
            base_.setPanelProperty(zlgui::PanelSettingIdx::kDynamicExtraPanel, 0.0);
        }
    }

    void ControlPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kMatchPanel, property)) {
            base_.setSelectedBand(zlp::kBandNum);
            const auto f = static_cast<int>(std::round(
                static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kMatchPanel))));
            match_control_panel_.setVisible(f > 0);
        }
    }
}
