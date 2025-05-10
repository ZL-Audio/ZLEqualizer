// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_control_panel.hpp"

namespace zlpanel {
    MatchControlPanel::MatchControlPanel(PluginProcessor &p, zlgui::UIBase &base)
        : ui_base_(base), analyzer_(p.getController().getMatchAnalyzer()),
          start_drawable_(juce::Drawable::createFromImageData(BinaryData::playfill_svg, BinaryData::playfill_svgSize)),
          pause_drawable_(juce::Drawable::createFromImageData(BinaryData::pauseline_svg, BinaryData::pauseline_svgSize)),
          save_drawable_(juce::Drawable::createFromImageData(BinaryData::saveline_svg, BinaryData::saveline_svgSize)),
          side_choose_box_("", {"Side", "Preset", "Flat"}, base, zlgui::multilingual::Labels::kMatchTarget),
          fit_algo_box_("", {"LD", "GN", "GN+"}, base, zlgui::multilingual::Labels::kMatchAlgo),
          weight_slider_("Weight", base, zlgui::multilingual::Labels::kMatchWeight),
          smooth_slider_("Smooth", base, zlgui::multilingual::Labels::kMatchSmooth),
          slope_slider_("Slope", base, zlgui::multilingual::Labels::kMatchSlope),
          num_band_slider_("Num Band", base, zlgui::multilingual::Labels::kMatchNumBand),
          learn_button_(base, start_drawable_.get(), pause_drawable_.get(),
                      zlgui::multilingual::Labels::kMatchStartLearn),
          save_button_(base, save_drawable_.get(), nullptr, zlgui::multilingual::Labels::kMatchSave),
          fit_button_(base, start_drawable_.get(), nullptr, zlgui::multilingual::Labels::kMatchStartFit),
          match_runner_(p, ui_base_, analyzer_.getDiffs(), num_band_slider_) {
        ui_base_.getValueTree().addListener(this);
        // create the preset directory if not exists
        if (!kPresetDirectory.isDirectory()) {
            const auto f = kPresetDirectory.createDirectory();
            juce::ignoreUnused(f);
        }
        // init combobox
        const auto *menu = side_choose_box_.getBox().getRootMenu();
        juce::PopupMenu::MenuItemIterator iterator(*menu);
        while (iterator.next()) {
            auto item = &iterator.getItem();
            if (item->itemID == 1) {
                item->setAction([this] {
                    analyzer_.setMatchMode(zldsp::eq_match::EqMatchAnalyzer<double>::kMatchSide);
                });
            } else if (item->itemID == 2) {
                item->setAction([this] {
                    loadFromPreset();
                    analyzer_.setMatchMode(zldsp::eq_match::EqMatchAnalyzer<double>::kMatchPreset);
                });
            } else if (item->itemID == 3) {
                item->setAction([this] {
                    analyzer_.setTargetSlope(0.f);
                    analyzer_.setMatchMode(zldsp::eq_match::EqMatchAnalyzer<double>::kMatchSlope);
                });
            }
        }
        fit_algo_box_.getBox().onChange = [this]() {
            const auto fit_algo = static_cast<size_t>(fit_algo_box_.getBox().getSelectedId() - 1);
            match_runner_.setMode(fit_algo);
        };
        for (const auto &c: {&side_choose_box_, &fit_algo_box_}) {
            addAndMakeVisible(c);
        }
        weight_slider_.getSlider().setRange(0.0, 1.0, 0.01);
        weight_slider_.getSlider().setDoubleClickReturnValue(true, .5);
        weight_slider_.getSlider().onValueChange = [this]() {
            analyzer_.getAverageFFT().setWeight(static_cast<float>(weight_slider_.getSlider().getValue()));
        };
        smooth_slider_.getSlider().setRange(0.0, 1.0, 0.01);
        smooth_slider_.getSlider().setDoubleClickReturnValue(true, .5);
        smooth_slider_.getSlider().onValueChange = [this]() {
            analyzer_.setSmooth(static_cast<float>(smooth_slider_.getSlider().getValue()));
        };
        slope_slider_.getSlider().setRange(-4.5, 4.5, 0.01);
        slope_slider_.getSlider().setDoubleClickReturnValue(true, 0.);
        slope_slider_.getSlider().onValueChange = [this]() {
            analyzer_.setSlope(static_cast<float>(slope_slider_.getSlider().getValue()));
        };
        num_band_slider_.getSlider().onValueChange = [this]() {
            match_runner_.setNumBand(static_cast<size_t>(num_band_slider_.getSlider().getValue()));
            match_runner_.update();
        };
        for (const auto &c: {
                 &weight_slider_, &smooth_slider_, &slope_slider_, &num_band_slider_
             }) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
        }
        for (const auto &c: {&learn_button_, &save_button_, &fit_button_}) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
            c->setPadding(.2f, .2f, .2f, .2f);
        }
        learn_button_.getButton().onClick = [this]() {
            analyzer_.setON(learn_button_.getButton().getToggleState());
            ui_base_.setProperty(zlgui::SettingIdx::kMatchPanelFit, false);
        };
        save_button_.getButton().onClick = [this]() {
            learn_button_.getButton().setToggleState(false, juce::dontSendNotification);
            analyzer_.setON(false);
            saveToPreset();
        };
        fit_button_.getButton().onClick = [this]() {
            learn_button_.getButton().setToggleState(false, juce::dontSendNotification);
            analyzer_.setON(false);
            match_runner_.start();
            ui_base_.setProperty(zlgui::SettingIdx::kMatchPanelFit, true);
            ui_base_.setProperty(zlgui::SettingIdx::kMatchFitRunning, true);
        };
        resetDefault();

        setOpaque(true);
    }

    MatchControlPanel::~MatchControlPanel() {
        ui_base_.getValueTree().removeListener(this);
        analyzer_.setON(false);
    }

    void MatchControlPanel::paint(juce::Graphics &g) {
        g.fillAll(ui_base_.getColourByIdx(zlgui::ColourIdx::kBackgroundColour));
        ui_base_.fillRoundedShadowRectangle(g,
                                          internal_bound_.toFloat(),
                                          0.5f * ui_base_.getFontSize(),
                                          {.blur_radius = 0.25f});
    }

    void MatchControlPanel::resized() {
        // update padding
        {
            for (const auto &c: {
                     &weight_slider_, &smooth_slider_, &slope_slider_, &num_band_slider_
                 }) {
                c->setPadding(std::round(ui_base_.getFontSize() * 0.5f),
                              std::round(ui_base_.getFontSize() * 0.6f));
            }
        }
        // update bounds
        auto bound = getLocalBounds();
        const auto button_width = static_cast<int>(ui_base_.getFontSize() * kButtonWidthP);
        const auto slider_width = static_cast<int>(std::round(ui_base_.getFontSize() * kRotarySliderWidthP * 1.1f)); {
            const auto pad = static_cast<int>(ui_base_.getFontSize() * .5f);
            internal_bound_ = bound.withSizeKeepingCentre(button_width + slider_width * 3 + 2 * pad, bound.getHeight());
            bound = internal_bound_;
            bound = bound.withSizeKeepingCentre(bound.getWidth() - 2 * pad, bound.getHeight() - 2 * pad);
        }
        const auto button_height = std::min(static_cast<int>(ui_base_.getFontSize() * kButtonHeightP),
                                           bound.getHeight() / 2); {
            auto m_bound = bound.removeFromLeft(slider_width);
            side_choose_box_.setBounds(m_bound.removeFromTop(button_height));
            weight_slider_.setBounds(m_bound.removeFromBottom(button_height));
        } {
            auto m_bound = bound.removeFromLeft(button_width);
            learn_button_.setBounds(m_bound.removeFromTop(button_height));
            save_button_.setBounds(m_bound.removeFromBottom(button_height));
        } {
            auto m_bound = bound.removeFromLeft(slider_width);
            smooth_slider_.setBounds(m_bound.removeFromTop(button_height));
            slope_slider_.setBounds(m_bound.removeFromBottom(button_height));
        } {
            auto m_bound = bound.removeFromLeft(slider_width);
            num_band_slider_.setBounds(m_bound.removeFromBottom(button_height));
            m_bound = m_bound.removeFromTop(button_height);
            fit_algo_box_.setBounds(m_bound.removeFromLeft(button_width));
            fit_button_.setBounds(m_bound.removeFromRight(button_width));
        }
    }

    void MatchControlPanel::resetDefault() {
        weight_slider_.getSlider().setValue(0.5, juce::dontSendNotification);
        weight_slider_.updateDisplayValue();
        analyzer_.getAverageFFT().setWeight(.5f);

        smooth_slider_.getSlider().setValue(0.5, juce::dontSendNotification);
        smooth_slider_.updateDisplayValue();
        analyzer_.setSmooth(.5f);

        slope_slider_.getSlider().setValue(0., juce::dontSendNotification);
        slope_slider_.updateDisplayValue();
        analyzer_.setSlope(0.f);

        learn_button_.getButton().setToggleState(false, juce::dontSendNotification);
        analyzer_.setON(false);
        analyzer_.reset();

        num_band_slider_.getSlider().setRange(1.0, 16.0, 1.0);
        num_band_slider_.getSlider().setValue(8.0, juce::dontSendNotification);
        num_band_slider_.updateDisplayValue();
        match_runner_.setNumBand(static_cast<size_t>(8));

        side_choose_box_.getBox().setSelectedId(1, juce::dontSendNotification);
        analyzer_.setMatchMode(zldsp::eq_match::EqMatchAnalyzer<double>::MatchMode::kMatchSide);

        fit_algo_box_.getBox().setSelectedId(2, juce::dontSendNotification);
        match_runner_.setMode(static_cast<size_t>(1));
    }

    void MatchControlPanel::valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &) {
        const auto f = static_cast<bool>(ui_base_.getProperty(zlgui::SettingIdx::kMatchPanelShow));
        setVisible(f);
        if (!f)  resetDefault();
    }

    void MatchControlPanel::loadFromPreset() {
        chooser_ = std::make_unique<juce::FileChooser>(
            "Load the match preset...", kPresetDirectory, "*.csv",
            true, false, nullptr);
        constexpr auto settingOpenFlags = juce::FileBrowserComponent::openMode |
                                          juce::FileBrowserComponent::canSelectFiles;
        chooser_->launchAsync(settingOpenFlags, [this](const juce::FileChooser &chooser) {
            if (chooser.getResults().size() <= 0) { return; }
            const juce::File settingFile(chooser.getResult());
            if (!settingFile.existsAsFile()) { return; }
            const auto stream(settingFile.createInputStream());
            if (stream->isExhausted()) { return; }
            const auto start = stream->readNextLine();
            if (start.startsWith("#native")) {
                std::array<float, 251> points{};
                size_t idx = 0;
                while (!stream->isExhausted() && idx < points.size()) {
                    points[idx] = stream->readNextLine().getFloatValue();
                    idx += 1;
                }
                if (idx == points.size()) {
                    analyzer_.setTargetPreset(points);
                }
            }
        });
    }

    void MatchControlPanel::saveToPreset() {
        chooser_ = std::make_unique<juce::FileChooser>(
            "Save the match preset...", kPresetDirectory.getChildFile("match.csv"), "*.csv",
            true, false, nullptr);
        constexpr auto setting_save_flags = juce::FileBrowserComponent::saveMode |
                                          juce::FileBrowserComponent::warnAboutOverwriting;
        chooser_->launchAsync(setting_save_flags, [this](const juce::FileChooser &chooser) {
            if (chooser.getResults().size() <= 0) { return; }
            juce::File settingFile(chooser.getResult().withFileExtension("csv"));
            if (settingFile.existsAsFile()) {
                const auto f = settingFile.deleteFile();
                juce::ignoreUnused(f);
            }
            const auto stream = settingFile.createOutputStream();
            stream->writeText("#native", false, false, nullptr);
            stream->writeText(",\n", false, false, nullptr);
            for (auto &p: analyzer_.getTarget()) {
                stream->writeText(juce::String(p.load()), false, false, nullptr);
                stream->writeText(",\n", false, false, nullptr);
            }
        });
    }
} // zlpanel
