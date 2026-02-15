// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "other_ui_setting_panel.hpp"

namespace zlpanel {
    OtherUISettingPanel::OtherUISettingPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p),
        base_(base), name_laf_(base),
        refresh_rate_box_(zlstate::PTargetRefreshSpeed::kChoices, base),
        fft_tilt_slider_("Tilt", base),
        fft_speed_slider_("Speed", base),
        single_curve_slider_("Single", base),
        sum_curve_slider_("Sum", base),
        tooltip_box_(zlstate::PTooltipLang::kChoices, base),
        font_mode_box_(zlstate::PFontMode::kChoices, base),
        font_scale_slider_("Scale", base),
        static_font_size_slider_("Static", base),
        window_size_fix_box_(zlstate::PWindowSizeFix::kChoices, base) {
        juce::ignoreUnused(p_ref_);
        name_laf_.setFontScale(zlgui::kFontHuge);

        refresh_rate_label_.setText("Refresh Rate", juce::dontSendNotification);
        refresh_rate_label_.setJustificationType(juce::Justification::centredRight);
        refresh_rate_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(refresh_rate_label_);
        addAndMakeVisible(refresh_rate_box_);

        fft_label_.setText("FFT", juce::dontSendNotification);
        fft_label_.setJustificationType(juce::Justification::centredRight);
        fft_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(fft_label_);
        fft_tilt_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(-4.5, 4.5, .01));
        fft_tilt_slider_.getSlider().setDoubleClickReturnValue(true, 0.);
        addAndMakeVisible(fft_tilt_slider_);
        fft_speed_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(0., 2., .01));
        fft_speed_slider_.getSlider().setDoubleClickReturnValue(true, 1.0);
        addAndMakeVisible(fft_speed_slider_);

        curve_thick_label_.setText("Curve Thickness", juce::dontSendNotification);
        curve_thick_label_.setJustificationType(juce::Justification::centredRight);
        curve_thick_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(curve_thick_label_);
        single_curve_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(0., 2., .01));
        single_curve_slider_.getSlider().setDoubleClickReturnValue(true, 1.0);
        addAndMakeVisible(single_curve_slider_);
        sum_curve_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(0., 2., .01));
        sum_curve_slider_.getSlider().setDoubleClickReturnValue(true, 1.0);
        addAndMakeVisible(sum_curve_slider_);

        tooltip_label_.setText("Tooltip", juce::dontSendNotification);
        tooltip_label_.setJustificationType(juce::Justification::centredRight);
        tooltip_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(tooltip_label_);
        addAndMakeVisible(tooltip_box_);

        font_label_.setText("UI Scaling", juce::dontSendNotification);
        font_label_.setJustificationType(juce::Justification::centredRight);
        font_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(font_label_);
        font_mode_box_.getBox().addListener(this);
        addAndMakeVisible(font_mode_box_);
        font_scale_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(0.5, 1.0, .01));
        font_scale_slider_.getSlider().setDoubleClickReturnValue(true, 0.9);
        addAndMakeVisible(font_scale_slider_);
        static_font_size_slider_.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(static_font_size_slider_);

        window_size_fix_label_.setText("Window Size Fix", juce::dontSendNotification);
        window_size_fix_label_.setJustificationType(juce::Justification::centredRight);
        window_size_fix_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(window_size_fix_label_);
        addAndMakeVisible(window_size_fix_box_);
    }

    void OtherUISettingPanel::loadSetting() {
        refresh_rate_box_.getBox().setSelectedItemIndex(static_cast<int>(base_.getRefreshRateID()));
        fft_tilt_slider_.getSlider().setValue(static_cast<double>(base_.getFFTExtraTilt()));
        fft_speed_slider_.getSlider().setValue(static_cast<double>(base_.getFFTExtraSpeed()));
        single_curve_slider_.getSlider().setValue(base_.getSingleEQCurveThickness());
        sum_curve_slider_.getSlider().setValue(base_.getSumEQCurveThickness());
        tooltip_box_.getBox().setSelectedItemIndex(static_cast<int>(base_.getTooltipLangID()));
        font_mode_box_.getBox().setSelectedItemIndex(static_cast<int>(base_.getFontMode()), juce::sendNotificationSync);
        font_scale_slider_.getSlider().setValue(static_cast<double>(base_.getFontScale()));
        static_font_size_slider_.getSlider().setValue(static_cast<double>(base_.getFontSize()));
        window_size_fix_box_.getBox().setSelectedItemIndex(static_cast<int>(base_.getWindowSizeFix()));
        comboBoxChanged(&font_mode_box_.getBox());
    }

    void OtherUISettingPanel::saveSetting() {
        base_.setRefreshRateID(static_cast<size_t>(refresh_rate_box_.getBox().getSelectedItemIndex()));
        base_.setFFTExtraTilt(static_cast<float>(fft_tilt_slider_.getSlider().getValue()));
        base_.setFFTExtraSpeed(static_cast<float>(fft_speed_slider_.getSlider().getValue()));
        base_.setSingleEQCurveThickness(static_cast<float>(single_curve_slider_.getSlider().getValue()));
        base_.setSumEQCurveThickness(static_cast<float>(sum_curve_slider_.getSlider().getValue()));
        base_.setTooltipLandID(static_cast<size_t>(tooltip_box_.getBox().getSelectedItemIndex()));
        base_.setFontMode(static_cast<size_t>(font_mode_box_.getBox().getSelectedItemIndex()));
        base_.setFontScale(static_cast<float>(font_scale_slider_.getSlider().getValue()));
        base_.setStaticFontSize(static_cast<float>(static_font_size_slider_.getSlider().getValue()));
        base_.setWindowSizeFix(window_size_fix_box_.getBox().getSelectedItemIndex() > 0);
        base_.saveToAPVTS();
    }

    void OtherUISettingPanel::resetSetting() {
    }

    int OtherUISettingPanel::getIdealHeight() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale * 3.f);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        return 7 * padding + 6 * slider_height;
    }

    void OtherUISettingPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale * 3.f);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderWidthScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        auto bound = getLocalBounds();
        {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            refresh_rate_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            refresh_rate_box_.setBounds(local_bound.removeFromLeft(slider_width).reduced(0, padding / 3));
        }
        {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            fft_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            fft_tilt_slider_.setBounds(local_bound.removeFromLeft(slider_width));
            local_bound.removeFromLeft(padding);
            fft_speed_slider_.setBounds(local_bound.removeFromLeft(slider_width));
        }
        {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            curve_thick_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            single_curve_slider_.setBounds(local_bound.removeFromLeft(slider_width));
            local_bound.removeFromLeft(padding);
            sum_curve_slider_.setBounds(local_bound.removeFromLeft(slider_width));
        }
        {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            tooltip_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            tooltip_box_.setBounds(local_bound.removeFromLeft(slider_width).reduced(0, padding / 3));
        }
        {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            font_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            font_mode_box_.setBounds(local_bound.removeFromLeft(slider_width).reduced(0, padding / 3));
            local_bound.removeFromLeft(padding);
            font_scale_slider_.setBounds(local_bound.removeFromLeft(slider_width));
            local_bound.removeFromLeft(padding);
            static_font_size_slider_.setBounds(local_bound.removeFromLeft(slider_width));

            if (parent_width_ < 2) {
                static_font_size_slider_.setVisible(false);
                return;
            }
            static_font_size_slider_.setVisible(true);
            const auto max_font_size = std::floor(static_cast<float>(parent_width_) * kFontSizeOverWidth);
            const auto min_font_size = std::ceil(static_cast<float>(parent_width_) * kFontSizeOverWidth * .25f);
            static_font_size_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(
                min_font_size, max_font_size, 0.01));
            static_font_size_slider_.getSlider().setDoubleClickReturnValue(
                true, .5f * (min_font_size + max_font_size));
        }
        {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            window_size_fix_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            window_size_fix_box_.setBounds(local_bound.removeFromLeft(slider_width).reduced(0, padding / 3));
        }
    }

    void OtherUISettingPanel::setParentWidth(const int width) {
        parent_width_ = width;
    }

    void OtherUISettingPanel::comboBoxChanged(juce::ComboBox*) {
        if (font_mode_box_.getBox().getSelectedItemIndex() == 0) {
            font_scale_slider_.setInterceptsMouseClicks(true, true);
            font_scale_slider_.setAlpha(1.f);
            static_font_size_slider_.setInterceptsMouseClicks(false, false);
            static_font_size_slider_.setAlpha(.5f);
        } else {
            font_scale_slider_.setInterceptsMouseClicks(false, false);
            font_scale_slider_.setAlpha(.5f);
            static_font_size_slider_.setInterceptsMouseClicks(true, true);
            static_font_size_slider_.setAlpha(1.f);
        }
    }
}
