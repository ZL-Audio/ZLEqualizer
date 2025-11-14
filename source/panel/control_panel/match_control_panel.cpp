// Copyright (C) 2025 - zsliu98
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
    MatchControlPanel::MatchControlPanel(PluginProcessor& p,
                                         zlgui::UIBase& base,
                                         const multilingual::TooltipHelper& tooltip_helper) :
        p_ref_(p), base_(base),
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
        smooth_label_("", "Smooth"),
        smooth_slider_("", base,
                       tooltip_helper.getToolTipText(multilingual::kEQMatchDiffSmooth)),
        slope_label_("", "Slope"),
        slope_slider_("", base,
                      tooltip_helper.getToolTipText(multilingual::kEQMatchDiffSlope)),
        start_drawable_(juce::Drawable::createFromImageData(BinaryData::start_svg,
                                                            BinaryData::start_svgSize)),
        fit_start_button_(base, start_drawable_.get(), nullptr,
                          tooltip_helper.getToolTipText(multilingual::kEQMatchFit)),

        num_band_slider_("", base,
                         tooltip_helper.getToolTipText(multilingual::kEQMatchNumBand)),
        match_runner_(p, base, num_band_slider_) {
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
            base_.setPanelProperty(zlgui::PanelSettingIdx::kMatchDrawing,
                                   static_cast<double>(draw_button_.getToggleState()));
        };
        draw_button_.setBufferedToImage(true);
        addAndMakeVisible(draw_button_);

        target_box_.getBox().onChange = [this]() {
            const auto mode = static_cast<zldsp::eq_match::MatchMode>(target_box_.getBox().getSelectedItemIndex());
            if (mode == zldsp::eq_match::MatchMode::kMatchPreset) {
                loadFromPreset();
            }
            p_ref_.getController().getEQMatchAnalyzer().setMatchMode(mode);
        };
        target_box_.setBufferedToImage(true);
        addAndMakeVisible(target_box_);

        shift_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(
            -30.0, 30.0, 0.1));
        shift_slider_.getSlider().setDoubleClickReturnValue(true, 0.);
        shift_slider_.getSlider().onValueChange = [this]() {
            p_ref_.getController().getEQMatchAnalyzer().setDiffShift(
                static_cast<float>(shift_slider_.getSlider().getValue()));
        };

        smooth_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(
            0.0, 1.0, 0.01));
        smooth_slider_.getSlider().setDoubleClickReturnValue(true, 0.5);
        smooth_slider_.getSlider().onValueChange = [this]() {
            p_ref_.getController().getEQMatchAnalyzer().setDiffSmooth(
                static_cast<float>(smooth_slider_.getSlider().getValue()));
        };

        slope_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(
            -4.5, 4.5, 0.1));
        slope_slider_.getSlider().setDoubleClickReturnValue(true, 0.);
        slope_slider_.getSlider().onValueChange = [this]() {
            p_ref_.getController().getEQMatchAnalyzer().setDiffSlope(
                static_cast<float>(slope_slider_.getSlider().getValue()));
        };

        label_laf_.setFontScale(1.5f);
        for (auto& l : {&shift_label_, &smooth_label_, &slope_label_}) {
            l->setLookAndFeel(&label_laf_);
            l->setJustificationType(juce::Justification::centred);
            l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        for (auto& s : {&shift_slider_, &smooth_slider_, &slope_slider_}) {
            s->setBufferedToImage(true);
            addAndMakeVisible(s);
        }

        fit_start_button_.getButton().onClick = [this]() {
            if (match_runner_.isThreadRunning()) {
                return;
            }
            num_band_slider_.setAlpha(.5f);
            num_band_slider_.setInterceptsMouseClicks(false, false);
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                const auto band_s = std::to_string(band);
                auto* status_para = p_ref_.parameters_.getParameter(zlp::PFilterStatus::kID + band_s);
                updateValue(status_para, 0.f);
                auto* stereo_para = p_ref_.parameters_.getParameter(zlp::PLRMode::kID + band_s);
                updateValue(stereo_para, 0.f);
                auto* dynamic_para = p_ref_.parameters_.getParameter(zlp::PDynamicON::kID + band_s);
                updateValue(dynamic_para, 0.f);
                band_helper::turnOnOffDynamic(p_ref_, band, false);
            }
            base_.setPanelProperty(zlgui::PanelSettingIdx::kMatchPanel, 2.0);
            match_runner_.startThread(juce::Thread::Priority::low);
        };
        fit_start_button_.setBufferedToImage(true);
        addAndMakeVisible(fit_start_button_);

        num_band_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(
            0.0, static_cast<double>(zlp::kBandNum), 1.0));
        num_band_slider_.setBufferedToImage(true);
        addAndMakeVisible(num_band_slider_);

        preset_freqs_.reserve(zlp::Controller::kAnalyzerPointNum);
        preset_dbs_.reserve(zlp::Controller::kAnalyzerPointNum);
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
                smooth_label_.setBounds(t_bound.removeFromLeft(small_slider_width));
                smooth_slider_.setBounds(t_bound);
                smooth_slider_.getSlider().setMouseDragSensitivity(small_slider_width);
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
        auto& analyzer{p_ref_.getController().getEQMatchAnalyzer()};
        target_box_.getBox().setSelectedItemIndex(0, juce::dontSendNotification);
        analyzer.setMatchMode(zldsp::eq_match::MatchMode::kMatchSide);
        smooth_slider_.getSlider().setValue(0.5, juce::sendNotificationSync);
        analyzer.setDiffSmooth(0.5f);
        slope_slider_.getSlider().setValue(0., juce::sendNotificationSync);
        analyzer.setDiffSlope(0.f);
        num_band_slider_.setAlpha(.5f);
        num_band_slider_.setInterceptsMouseClicks(false, false);
        num_band_slider_.getSlider().setValue(0., juce::dontSendNotification);
        num_band_slider_.updateDisplayValue();

        draw_button_.getButton().setToggleState(true, juce::sendNotificationSync);
        base_.setPanelProperty(zlgui::PanelSettingIdx::kMatchDrawing, 1.0);
    }

    void MatchControlPanel::saveToPreset() {
        chooser_ = std::make_unique<juce::FileChooser>(
            "Save the match preset...", kPresetDirectory.getChildFile("match.csv"), "*.csv",
            true, false, nullptr);
        constexpr auto setting_save_flags = juce::FileBrowserComponent::saveMode |
            juce::FileBrowserComponent::warnAboutOverwriting;
        chooser_->launchAsync(setting_save_flags, [this](const juce::FileChooser& chooser) {
            if (chooser.getResults().size() <= 0) { return; }
            auto& analyzer{p_ref_.getController().getEQMatchAnalyzer()};
            analyzer.saveFreq(preset_freqs_);
            analyzer.saveTarget(preset_dbs_);
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
                auto& analyzer{p_ref_.getController().getEQMatchAnalyzer()};
                analyzer.setTargetPreset(preset_freqs_, preset_dbs_);
            }
        });
    }
}
