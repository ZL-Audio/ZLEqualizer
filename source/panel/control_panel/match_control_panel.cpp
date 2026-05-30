// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_control_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    MatchControlPanel::MatchControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                         MatchFFTPanel& match_fft_panel,
                                         const multilingual::TooltipHelper& tooltip_helper) :
        p_ref_(p), base_(base),
        match_fft_panel_(match_fft_panel),
        control_background_(base),
        save_drawable_(juce::Drawable::createFromImageData(BinaryData::save_svg,
                                                           BinaryData::save_svgSize)),
        save_button_(base, save_drawable_.get(), nullptr,
                     tooltip_helper.getToolTipText(multilingual::kEQMatchSave)),
        draw_drawable_(juce::Drawable::createFromImageData(BinaryData::draw_svg,
                                                           BinaryData::draw_svgSize)),
        draw_button_(base, draw_drawable_.get(), draw_drawable_.get(),
                     tooltip_helper.getToolTipText(multilingual::kEQMatchDiffDraw)),
        target_box_({"Side", "Flat", "Preset"}, base,
                    tooltip_helper.getToolTipText(multilingual::kEQMatchTarget),
                    {tooltip_helper.getToolTipText(multilingual::kEQMatchTargetSide),
                     tooltip_helper.getToolTipText(multilingual::kEQMatchTargetFlat),
                     tooltip_helper.getToolTipText(multilingual::kEQMatchTargetPreset)}),
        label_laf_(base),
        shift_label_("", "Shift"),
        shift_slider_("", base,
                      tooltip_helper.getToolTipText(multilingual::kEQMatchDiffShift)),
        scale_label_("", "Scale"),
        scale_slider_("", base,
                      tooltip_helper.getToolTipText(multilingual::kEQMatchDiffSmooth)),
        slope_label_("", "Slope"),
        slope_slider_("", base,
                      tooltip_helper.getToolTipText(multilingual::kEQMatchDiffSlope)),
        start_drawable_(juce::Drawable::createFromImageData(BinaryData::start_svg,
                                                            BinaryData::start_svgSize)),
        fit_start_button_(base, start_drawable_.get(), nullptr,
                          tooltip_helper.getToolTipText(multilingual::kEQMatchFit)),
        num_band_slider_("", base,
                         tooltip_helper.getToolTipText(multilingual::kEQMatchNumBand)) {
        juce::ignoreUnused(p_ref_);
        // create the preset directory if not exists
        if (!kPresetDirectory.isDirectory()) {
            const auto f = kPresetDirectory.createDirectory();
            juce::ignoreUnused(f);
        }

        control_background_.setBufferedToImage(true);
        addAndMakeVisible(control_background_);

        save_button_.getButton().onClick = [this]() {
            saveToPreset();
        };
        save_button_.setBufferedToImage(true);
        addAndMakeVisible(save_button_);

        draw_button_.getButton().onClick = [this]() {
            match_fft_panel_.setDiffDrawOn(draw_button_.getToggleState());
        };
        draw_button_.setBufferedToImage(true);
        addAndMakeVisible(draw_button_);

        target_box_.getBox().onChange = [this]() {
            const auto mode = static_cast<MatchFFTPanel::SideMode>(target_box_.getBox().getSelectedItemIndex());
            if (mode == MatchFFTPanel::SideMode::kPreset) {
                loadFromPreset();
            }
            match_fft_panel_.setSideMode(mode);
        };
        target_box_.setBufferedToImage(true);
        addAndMakeVisible(target_box_);

        shift_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(
            -30.0, 30.0, 0.1));
        shift_slider_.getSlider().setDoubleClickReturnValue(true, 0.);
        shift_slider_.getSlider().onValueChange = [this]() {
            match_fft_panel_.setDiffShift(static_cast<float>(shift_slider_.getSlider().getValue()));
        };

        scale_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(
            0.0, 1.0, 0.01));
        scale_slider_.getSlider().setDoubleClickReturnValue(true, 1.0);
        scale_slider_.getSlider().onValueChange = [this]() {
            match_fft_panel_.setDiffScale(static_cast<float>(scale_slider_.getSlider().getValue()));
        };

        slope_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(
            -4.5, 4.5, 0.1));
        slope_slider_.getSlider().setDoubleClickReturnValue(true, 0.);
        slope_slider_.getSlider().onValueChange = [this]() {
            match_fft_panel_.setDiffSlope(static_cast<float>(slope_slider_.getSlider().getValue()));
        };

        label_laf_.setFontScale(1.5f);
        for (auto& l : {&shift_label_, &scale_label_, &slope_label_}) {
            l->setLookAndFeel(&label_laf_);
            l->setJustificationType(juce::Justification::centred);
            l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        for (auto& s : {&shift_slider_, &scale_slider_, &slope_slider_}) {
            s->setBufferedToImage(true);
            addAndMakeVisible(s);
        }

        fit_start_button_.getButton().onClick = [this]() {
            base_.setPanelProperty(zlgui::PanelSettingIdx::kSuggestedNumBand, 0.0);
            base_.setPanelProperty(zlgui::PanelSettingIdx::kMatchPanel, 2.0);
            match_fft_panel_.setMatchPhase(MatchFFTPanel::MatchPhase::kMatch);
        };
        fit_start_button_.setBufferedToImage(true);
        addAndMakeVisible(fit_start_button_);

        num_band_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(
            0.0, static_cast<double>(zlp::kBandNum), 1.0));
        num_band_slider_.getSlider().onValueChange = [this]() {
            match_fft_panel_.updateMatchNumBand(static_cast<size_t>(
                std::round(num_band_slider_.getSlider().getValue())));
        };
        num_band_slider_.setBufferedToImage(true);
        addChildComponent(num_band_slider_);

        base_.getPanelValueTree().addListener(this);
    }

    MatchControlPanel::~MatchControlPanel() {
        base_.getPanelValueTree().removeListener(this);
    }

    int MatchControlPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        const auto button_size = getButtonSize(font_size);
        const auto padding = getPaddingSize(font_size);

        return 4 * padding + 2 * (padding / 2) + 3 * button_size;
    }

    int MatchControlPanel::getIdealWidth() const {
        const auto font_size = base_.getFontSize();
        const auto small_slider_width = getSmallSliderWidth(font_size);
        const auto button_size = getButtonSize(font_size);
        const auto padding = getPaddingSize(font_size);

        return 4 * padding + 2 * (padding / 2) + 2 * small_slider_width + 4 * button_size;
    }

    void MatchControlPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto small_slider_width = getSmallSliderWidth(font_size);
        const auto box_height = getBoxHeight(font_size);
        const auto button_size = getButtonSize(font_size);
        const auto padding = getPaddingSize(font_size);

        auto bound = getLocalBounds();
        control_background_.setBounds(bound);

        bound.reduce(padding + padding / 2, padding + padding / 2);
        {
            auto temp_bound = bound.removeFromLeft(small_slider_width);
            const auto box_bound = temp_bound.removeFromBottom(temp_bound.getHeight() / 2);
            target_box_.setBounds(box_bound.withSizeKeepingCentre(box_bound.getWidth(), box_height));
            temp_bound.removeFromBottom(padding);
            const auto button_bound1 = temp_bound.removeFromLeft(temp_bound.getWidth() / 2);
            save_button_.setBounds(button_bound1.withSizeKeepingCentre(button_size, button_size));
            draw_button_.setBounds(temp_bound.withSizeKeepingCentre(button_size, button_size));
        }
        bound.removeFromLeft(padding);
        {
            auto temp_bound = bound.removeFromLeft(small_slider_width + 2 * button_size);
            {
                auto t_bound = temp_bound.removeFromTop(button_size);
                shift_label_.setBounds(t_bound.removeFromLeft(small_slider_width));
                shift_slider_.setBounds(t_bound);
                shift_slider_.getSlider().setMouseDragSensitivity(small_slider_width);
            }
            temp_bound.removeFromTop(padding);
            {
                auto t_bound = temp_bound.removeFromTop(button_size);
                scale_label_.setBounds(t_bound.removeFromLeft(small_slider_width));
                scale_slider_.setBounds(t_bound);
                scale_slider_.getSlider().setMouseDragSensitivity(small_slider_width);
            }
            temp_bound.removeFromTop(padding);
            {
                auto t_bound = temp_bound.removeFromTop(button_size);
                slope_label_.setBounds(t_bound.removeFromLeft(small_slider_width));
                slope_slider_.setBounds(t_bound);
                slope_slider_.getSlider().setMouseDragSensitivity(small_slider_width);
            }
        }
        bound.removeFromLeft(padding);
        {
            auto temp_bound = bound.removeFromLeft(2 * button_size);
            num_band_slider_.setBounds(temp_bound.removeFromBottom(temp_bound.getHeight() / 2));
            num_band_slider_.getSlider().setMouseDragSensitivity(small_slider_width);
            temp_bound.removeFromBottom(padding);
            fit_start_button_.setBounds(temp_bound.withSizeKeepingCentre(button_size, button_size));
        }
    }

    void MatchControlPanel::visibilityChanged() {
        if (!isVisible()) {
            return;
        }
        draw_button_.getButton().setToggleState(true, juce::sendNotificationSync);
        match_fft_panel_.setDiffDrawOn(true);
        target_box_.getBox().setSelectedItemIndex(0, juce::dontSendNotification);
        match_fft_panel_.setSideMode(MatchFFTPanel::SideMode::kSide);
        shift_slider_.getSlider().setValue(0.0, juce::sendNotificationSync);
        match_fft_panel_.setDiffShift(0.f);
        scale_slider_.getSlider().setValue(1.0, juce::sendNotificationSync);
        match_fft_panel_.setDiffScale(1.f);
        slope_slider_.getSlider().setValue(0.0, juce::sendNotificationSync);
        match_fft_panel_.setDiffSlope(0.f);
    }

    void MatchControlPanel::saveToPreset() {
        chooser_ = std::make_unique<juce::FileChooser>(
            "Save the match preset...", kPresetDirectory.getChildFile("match.csv"), "*.csv",
            true, false, nullptr);
        constexpr auto setting_save_flags = juce::FileBrowserComponent::saveMode |
            juce::FileBrowserComponent::warnAboutOverwriting;
        chooser_->launchAsync(setting_save_flags, [this](const juce::FileChooser& chooser) {
            if (chooser.getResults().size() <= 0) { return; }
            match_fft_panel_.saveToPreset(preset_freqs_, preset_dbs_);
            juce::File preset_file(chooser.getResult().withFileExtension("csv"));
            if (juce::FileOutputStream output(preset_file); output.openedOk()) {
                output.setPosition(0);
                output.truncate();
                for (size_t i = 0; i < preset_freqs_.size(); ++i) {
                    output.writeText(juce::String(preset_freqs_[i]), false, false, nullptr);
                    output.writeText(",", false, false, nullptr);
                    output.writeText(juce::String(preset_dbs_[i]), false, false, nullptr);
                    output.writeText("\n", false, false, nullptr);
                }
            }
        });
    }

    void MatchControlPanel::loadFromPreset() {
        chooser_ = std::make_unique<juce::FileChooser>(
            "Load the match preset...", kPresetDirectory, "*.csv",
            true, false, nullptr);
        constexpr auto settingOpenFlags = juce::FileBrowserComponent::openMode |
            juce::FileBrowserComponent::canSelectFiles;
        chooser_->launchAsync(settingOpenFlags, [this](const juce::FileChooser& chooser) {
            if (chooser.getResults().size() <= 0) { return; }
            const juce::File preset_file(chooser.getResult());
            if (!preset_file.existsAsFile()) { return; }
            if (juce::FileInputStream input(preset_file); input.openedOk()) {
                preset_freqs_.resize(0);
                preset_dbs_.resize(0);
                while (!input.isExhausted()) {
                    const auto line = input.readNextLine().toStdString();
                    try {
                        const size_t comma_pos = line.find(',');
                        if (comma_pos == std::string::npos) {
                            break;
                        }
                        std::string freq_str = line.substr(0, comma_pos);
                        std::string db_str = line.substr(comma_pos + 1);
                        float freq = std::stof(freq_str);
                        float db = std::stof(db_str);
                        preset_freqs_.emplace_back(freq);
                        preset_dbs_.emplace_back(db);
                    }
                    catch (const std::exception& e) {
                        break;
                    }
                }
                if (preset_freqs_.size() < 2) {
                    return;
                }
                match_fft_panel_.loadFromPreset(preset_freqs_, preset_dbs_);
            }
        });
    }

    void MatchControlPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kMaximumNumBand, property)) {
            const auto max_num_band = static_cast<double>(
                base_.getPanelProperty(zlgui::PanelSettingIdx::kMaximumNumBand));
            num_band_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(
                0.0, max_num_band, 1.0));
        } else if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kSuggestedNumBand, property)) {
            const auto suggested_num_band = std::round(static_cast<double>(
                base_.getPanelProperty(zlgui::PanelSettingIdx::kSuggestedNumBand)));
            num_band_slider_.getSlider().setDoubleClickReturnValue(true, suggested_num_band);
            num_band_slider_.getSlider().setValue(suggested_num_band, juce::sendNotificationSync);
        } else if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kMatchPanel, property)) {
            const auto is_finished = static_cast<double>(
                base_.getPanelProperty(zlgui::PanelSettingIdx::kMatchPanel)) > 2.5;
            num_band_slider_.setVisible(is_finished);
        }
    }
}
