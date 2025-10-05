// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "left_control_panel.hpp"

namespace zlpanel {
    LeftControlPanel::LeftControlPanel(PluginProcessor& p,
                                       zlgui::UIBase& base,
                                       const multilingual::TooltipHelper& tooltip_helper) :
        p_ref_(p), base_(base), updater_(),
        sub_left_control_panel_(p, base, tooltip_helper),
        freq_slider_("", base,
                     tooltip_helper.getToolTipText(multilingual::kBandFreq)),
        gain_slider_("", base,
                     tooltip_helper.getToolTipText(multilingual::kBandGain)) {
        addAndMakeVisible(freq_slider_);
        gain_slider_.setShowSlider2(false);
        addAndMakeVisible(gain_slider_);
        setInterceptsMouseClicks(false, true);
        setBufferedToImage(true);
    }

    LeftControlPanel::~LeftControlPanel() = default;

    int LeftControlPanel::getIdealWidth() const {
        return sub_left_control_panel_.getIdealWidth();
    }

    void LeftControlPanel::resized() {
        auto bound = getLocalBounds();
        sub_left_control_panel_.setBounds(bound);

        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);

        bound.reduce(padding, padding);
        bound.removeFromTop(button_height);
        bound.removeFromLeft(padding + slider_width);
        freq_slider_.setBounds(bound.removeFromLeft(slider_width));
        bound.removeFromLeft(padding);
        gain_slider_.setBounds(bound.removeFromLeft(slider_width));
    }

    void LeftControlPanel::repaintCallBackSlow() {
        if (dynamic_on_ptr_ != nullptr) {
            const auto dynamic_on = dynamic_on_ptr_->load(std::memory_order::relaxed) > .5f;
            if (dynamic_on != c_dynamic_on_) {
                c_dynamic_on_ = dynamic_on;
                gain_slider_.setShowSlider2(dynamic_on);
            }
        }
        updater_.updateComponents();
        sub_left_control_panel_.repaintCallBackSlow();
    }

    void LeftControlPanel::updateBand() {
        if (base_.getSelectedBand() < zlp::kBandNum) {
            const auto band_s = std::to_string(base_.getSelectedBand());
            dynamic_on_ptr_ = p_ref_.parameters_.getRawParameterValue(zlp::PDynamicON::kID + band_s);
            updateFreqMax(freq_max_);
            gain_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                gain_slider_.getSlider1(), p_ref_.parameters_, zlp::PGain::kID + band_s, updater_);
            target_gain_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                gain_slider_.getSlider2(), p_ref_.parameters_, zlp::PTargetGain::kID + band_s, updater_);
        } else {
            dynamic_on_ptr_ = nullptr;
            freq_attachment_.reset();
            gain_attachment_.reset();
            target_gain_attachment_.reset();
        }
        sub_left_control_panel_.updateBand();
    }

    void LeftControlPanel::updateFreqMax(const double freq_max) {
        freq_max_ = freq_max;
        if (base_.getSelectedBand() < zlp::kBandNum) {
            freq_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                freq_slider_.getSlider1(), p_ref_.parameters_,
                zlp::PFreq::kID + std::to_string(base_.getSelectedBand()),
                zlp::getLogMidRange(10.0, freq_max, 1000.0, 0.1),
                updater_);
        } else {
            freq_attachment_.reset();
        }
    }
}
