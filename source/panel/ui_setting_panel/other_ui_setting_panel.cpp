// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "other_ui_setting_panel.hpp"

namespace zlpanel {
    OtherUISettingPanel::OtherUISettingPanel(PluginProcessor &p, zlgui::UIBase &base)
        : pRef(p),
          ui_base_(base), name_laf_(base),
          rendering_engine_box_("", zlstate::renderingEngine::choices, base),
          refresh_rate_box_("", zlstate::refreshRate::choices, base),
          fft_tilt_slider_("Tilt", base),
          fft_speed_slider_("Speed", base),
          fft_order_box_("order", zlstate::ffTOrder::choices, base),
          single_curve_slider_("Single", base),
          sum_curve_slider_("Sum", base),
          default_pass_filter_slope_box_("", zlstate::defaultPassFilterSlope::choices, base),
          dyn_link_box_("", zlstate::dynLink::choices, base),
          tooltip_on_box_("", zlstate::tooltipON::choices, base),
          tooltip_lang_box_("", zlstate::tooltipLang::choices, base) {
        juce::ignoreUnused(pRef);
        name_laf_.setFontScale(zlgui::kFontHuge);
        rendering_engine_label_.setText("Rendering Engine", juce::dontSendNotification);
        rendering_engine_label_.setJustificationType(juce::Justification::centredRight);
        rendering_engine_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(rendering_engine_label_);
        addAndMakeVisible(rendering_engine_box_);
        refresh_rate_label_.setText("Refresh Rate", juce::dontSendNotification);
        refresh_rate_label_.setJustificationType(juce::Justification::centredRight);
        refresh_rate_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(refresh_rate_label_);
        addAndMakeVisible(refresh_rate_box_);
        fft_label_.setText("FFT", juce::dontSendNotification);
        fft_label_.setJustificationType(juce::Justification::centredRight);
        fft_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(fft_label_);
        fft_tilt_slider_.getSlider().setNormalisableRange(zlstate::fftExtraTilt::doubleRange);
        fft_tilt_slider_.getSlider().setDoubleClickReturnValue(true, static_cast<double>(zlstate::fftExtraTilt::defaultV));
        fft_speed_slider_.getSlider().setNormalisableRange(zlstate::fftExtraSpeed::doubleRange);
        fft_speed_slider_.getSlider().setDoubleClickReturnValue(
            true, static_cast<double>(zlstate::fftExtraSpeed::defaultV));
        addAndMakeVisible(fft_tilt_slider_);
        addAndMakeVisible(fft_speed_slider_);
        addAndMakeVisible(fft_order_box_);
        curve_thick_label_.setText("Curve Thickness", juce::dontSendNotification);
        curve_thick_label_.setJustificationType(juce::Justification::centredRight);
        curve_thick_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(curve_thick_label_);
        single_curve_slider_.getSlider().setNormalisableRange(zlstate::singleCurveThickness::doubleRange);
        single_curve_slider_.getSlider().setDoubleClickReturnValue(true, zlstate::singleCurveThickness::defaultV);
        sum_curve_slider_.getSlider().setNormalisableRange(zlstate::sumCurveThickness::doubleRange);
        sum_curve_slider_.getSlider().setDoubleClickReturnValue(true, zlstate::sumCurveThickness::defaultV);
        addAndMakeVisible(single_curve_slider_);
        addAndMakeVisible(sum_curve_slider_);
        default_pass_filter_slope_label_.setText("Default Pass Filter Slope", juce::dontSendNotification);
        default_pass_filter_slope_label_.setJustificationType(juce::Justification::centredRight);
        default_pass_filter_slope_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(default_pass_filter_slope_label_);
        addAndMakeVisible(default_pass_filter_slope_box_);
        dyn_link_label_.setText("Default Dynamic Link", juce::dontSendNotification);
        dyn_link_label_.setJustificationType(juce::Justification::centredRight);
        dyn_link_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(dyn_link_label_);
        addAndMakeVisible(dyn_link_box_);
        tooltip_label_.setText("Tooltip", juce::dontSendNotification);
        tooltip_label_.setJustificationType(juce::Justification::centredRight);
        tooltip_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(tooltip_label_);
        addAndMakeVisible(tooltip_on_box_);
        addAndMakeVisible(tooltip_lang_box_);
    }

    void OtherUISettingPanel::loadSetting() {
        rendering_engine_box_.getBox().setSelectedId(static_cast<int>(ui_base_.getRenderingEngine()) + 1);
        refresh_rate_box_.getBox().setSelectedId(static_cast<int>(ui_base_.getRefreshRateID()) + 1);
        fft_tilt_slider_.getSlider().setValue(static_cast<double>(ui_base_.getFFTExtraTilt()));
        fft_speed_slider_.getSlider().setValue(static_cast<double>(ui_base_.getFFTExtraSpeed()));
        fft_order_box_.getBox().setSelectedId(ui_base_.getFFTOrderIdx() + 1);
        single_curve_slider_.getSlider().setValue(ui_base_.getSingleCurveThickness());
        sum_curve_slider_.getSlider().setValue(ui_base_.getSumCurveThickness());
        default_pass_filter_slope_box_.getBox().setSelectedId(ui_base_.getDefaultPassFilterSlope() + 1);
        dyn_link_box_.getBox().setSelectedId(static_cast<int>(ui_base_.getDynLink()) + 1);
        tooltip_on_box_.getBox().setSelectedId(static_cast<int>(ui_base_.getTooltipON()) + 1);
        tooltip_lang_box_.getBox().setSelectedId(ui_base_.getLangIdx() + 1);
    }

    void OtherUISettingPanel::saveSetting() {
        ui_base_.setRenderingEngine(static_cast<int>(rendering_engine_box_.getBox().getSelectedId() - 1));
        ui_base_.setRefreshRateID(static_cast<size_t>(refresh_rate_box_.getBox().getSelectedId() - 1));
        ui_base_.setFFTExtraTilt(static_cast<float>(fft_tilt_slider_.getSlider().getValue()));
        ui_base_.setFFTExtraSpeed(static_cast<float>(fft_speed_slider_.getSlider().getValue()));
        ui_base_.setFFTOrderIdx(fft_order_box_.getBox().getSelectedId() - 1);
        ui_base_.setSingleCurveThickness(static_cast<float>(single_curve_slider_.getSlider().getValue()));
        ui_base_.setSumCurveThickness(static_cast<float>(sum_curve_slider_.getSlider().getValue()));
        ui_base_.setDefaultPassFilterSlope(default_pass_filter_slope_box_.getBox().getSelectedId() - 1);
        ui_base_.setDynLink(dyn_link_box_.getBox().getSelectedId() == 2);
        ui_base_.setTooltipON(tooltip_on_box_.getBox().getSelectedId() == 2);
        ui_base_.setLangIdx(tooltip_lang_box_.getBox().getSelectedId() - 1);
        ui_base_.saveToAPVTS();
    }

    void OtherUISettingPanel::resetSetting() {
    }

    void OtherUISettingPanel::resized() {
        auto bound = getLocalBounds().toFloat(); {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            rendering_engine_label_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .05f);
            const auto s_width = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.6f;
            rendering_engine_box_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            refresh_rate_label_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .05f);
            const auto s_width = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            refresh_rate_box_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            fft_label_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .05f);
            const auto s_width = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            fft_tilt_slider_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
            local_bound.removeFromLeft(ui_base_.getFontSize() * 2.f);
            fft_speed_slider_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
            local_bound.removeFromLeft(ui_base_.getFontSize() * 2.f);
            fft_order_box_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            curve_thick_label_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .05f);
            const auto s_width = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            single_curve_slider_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
            local_bound.removeFromLeft(ui_base_.getFontSize() * 2.f);
            sum_curve_slider_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            default_pass_filter_slope_label_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .05f);
            const auto s_width = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            default_pass_filter_slope_box_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            dyn_link_label_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .05f);
            const auto s_width = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            dyn_link_box_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            tooltip_label_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .05f);
            const auto s_width = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            tooltip_on_box_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
            local_bound.removeFromLeft(ui_base_.getFontSize() * 3.f);
            tooltip_lang_box_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
        }
    }

    void OtherUISettingPanel::setRendererList(const juce::StringArray &rendererList) {
        auto idx = rendering_engine_box_.getBox().getSelectedItemIndex();
        rendering_engine_box_.getBox().clear();
        rendering_engine_box_.getBox().addItemList(rendererList, 1);
        if (idx >= 0) {
            idx = std::min(idx, rendering_engine_box_.getBox().getNumItems() - 1);
            rendering_engine_box_.getBox().setSelectedItemIndex(idx);
        }
    }
} // zlpanel
