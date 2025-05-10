// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "curve_panel.hpp"

namespace zlpanel {
    CurvePanel::CurvePanel(PluginProcessor &processor,
                           zlgui::UIBase &base)
        : Thread("curve panel"),
          processor_ref_(processor),
          parameters_ref_(processor.parameters_), parameters_NA_ref_(processor.parameters_NA_), ui_base_(base),
          controller_ref_(processor.getController()),
          background_panel_(parameters_ref_, parameters_NA_ref_, base),
          fft_panel_(controller_ref_.getAnalyzer(), base),
          conflict_panel_(controller_ref_.getConflictAnalyzer(), base),
          sum_panel_(parameters_ref_, base, controller_ref_, base_filters_, main_filters_),
          loudness_display_(processor_ref_, base),
          button_panel_(processor_ref_, base),
          solo_panel_(parameters_ref_, parameters_NA_ref_, base, controller_ref_, button_panel_),
          match_panel_(processor.getController().getMatchAnalyzer(), parameters_NA_ref_, base) {
        for (auto &filters: {&base_filters_, &target_filters_, &main_filters_}) {
            for (auto &f: *filters) {
                f.prepare(48000.0);
                f.prepareDBSize(ws.size());
            }
        }
        addAndMakeVisible(background_panel_, 0);
        addAndMakeVisible(fft_panel_, 1);
        addChildComponent(conflict_panel_, 2);
        parameters_NA_ref_.addParameterListener(zlstate::selectedBandIdx::ID, this);
        parameterChanged(zlstate::selectedBandIdx::ID,
                         parameters_NA_ref_.getRawParameterValue(zlstate::selectedBandIdx::ID)->load());
        addAndMakeVisible(dummy_component_, 4);
        dummy_component_.setInterceptsMouseClicks(false, false);
        for (size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
            const auto i = idx;
            single_panels_[idx] =
                    std::make_unique<SinglePanel>(i, parameters_ref_, parameters_NA_ref_, base, controller_ref_,
                                                  base_filters_[i], target_filters_[i], main_filters_[i]);
            dummy_component_.addAndMakeVisible(*single_panels_[idx]);
        }
        for (size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
            const auto i = idx;
            side_panels_[idx] = std::make_unique<SidePanel>(i, parameters_ref_, parameters_NA_ref_, base, controller_ref_,
                                                          button_panel_.getSideDragger(i));
            addAndMakeVisible(*side_panels_[idx], 5);
        }
        addAndMakeVisible(sum_panel_, 6);
        addChildComponent(solo_panel_, 7);
        addAndMakeVisible(loudness_display_, 8);
        addAndMakeVisible(button_panel_, 9);
        addChildComponent(match_panel_, 10);
        parameterChanged(zlp::scale::ID, parameters_ref_.getRawParameterValue(zlp::scale::ID)->load());
        parameters_ref_.addParameterListener(zlp::scale::ID, this);
        parameterChanged(zlstate::maximumDB::ID, parameters_NA_ref_.getRawParameterValue(zlstate::maximumDB::ID)->load());
        parameters_NA_ref_.addParameterListener(zlstate::maximumDB::ID, this);
        parameterChanged(zlstate::minimumFFTDB::ID,
                         parameters_NA_ref_.getRawParameterValue(zlstate::minimumFFTDB::ID)->load());
        parameters_NA_ref_.addParameterListener(zlstate::minimumFFTDB::ID, this);
        startThread(juce::Thread::Priority::low);

        ui_base_.getValueTree().addListener(this);
    }

    CurvePanel::~CurvePanel() {
        ui_base_.getValueTree().removeListener(this);
        if (isThreadRunning()) {
            stopThread(-1);
        }
        parameters_ref_.removeParameterListener(zlp::scale::ID, this);
        parameters_NA_ref_.removeParameterListener(zlstate::selectedBandIdx::ID, this);
        parameters_NA_ref_.removeParameterListener(zlstate::maximumDB::ID, this);
        parameters_NA_ref_.removeParameterListener(zlstate::minimumFFTDB::ID, this);
    }

    void CurvePanel::paint(juce::Graphics &g) {
        juce::ignoreUnused(g);
        if (!ui_base_.getIsRenderingHardware()) {
            physical_pixel_scale_factor_.store(g.getInternalContext().getPhysicalPixelScaleFactor());
        }
    }

    void CurvePanel::paintOverChildren(juce::Graphics &g) {
        juce::ignoreUnused(g);
        if (to_notify_) {
            to_notify_ = false;
            notify();
        }
    }

    void CurvePanel::resized() {
        const auto bound = getLocalBounds();
        background_panel_.setBounds(bound);
        fft_panel_.setBounds(bound);
        conflict_panel_.setBounds(bound);
        dummy_component_.setBounds(bound);
        for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
            single_panels_[i]->setBounds(bound);
        }
        const auto sideBound = bound.toFloat().withTop(bound.toFloat().getBottom() - 2.f * ui_base_.getFontSize());
        for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
            side_panels_[i]->setBounds(sideBound.toNearestInt());
        }
        sum_panel_.setBounds(bound);
        solo_panel_.setBounds(bound);
        button_panel_.setBounds(bound);
        match_panel_.setBounds(bound);

        auto l_bound = getLocalBounds().toFloat();
        l_bound = juce::Rectangle<float>(l_bound.getX() + l_bound.getWidth() * 0.666f,
                                        l_bound.getBottom() - ui_base_.getFontSize() * .5f,
                                        l_bound.getWidth() * .09f, ui_base_.getFontSize() * .5f);
        loudness_display_.setBounds(l_bound.toNearestInt());
    }

    void CurvePanel::parameterChanged(const juce::String &parameter_id, const float new_value) {
        if (parameter_id == zlstate::selectedBandIdx::ID) {
            band_idx_.store(static_cast<size_t>(new_value));
        } else if (parameter_id == zlstate::maximumDB::ID) {
            const auto idx = static_cast<size_t>(new_value);
            const auto max_db = zlstate::maximumDB::dBs[idx];
            sum_panel_.setMaximumDB(max_db);
            for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
                single_panels_[i]->setMaximumDB(max_db);
            }
        } else if (parameter_id == zlp::scale::ID) {
            const auto scale = static_cast<double>(zlp::scale::formatV(new_value));
            for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
                single_panels_[i]->setScale(scale);
            }
        } else if (parameter_id == zlstate::minimumFFTDB::ID) {
            const auto idx = static_cast<size_t>(new_value);
            const auto min_db = zlstate::minimumFFTDB::dBs[idx];
            fft_panel_.setMinimumFFTDB(min_db);
        }
    }

    void CurvePanel::valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &property) {
        if (property == zlgui::kMatchIdentifiers[static_cast<size_t>(zlgui::SettingIdx::kMatchPanelShow)]) {
            const auto f = static_cast<bool>(ui_base_.getProperty(zlgui::SettingIdx::kMatchPanelShow));
            show_match_panel_.store(f);
            match_panel_.setVisible(f);
            button_panel_.setVisible(!f);
            loudness_display_.updateVisible(!f);
            if (f) { solo_panel_.turnOffSolo(); }
        } else if (property == zlgui::kMatchIdentifiers[static_cast<size_t>(
                       zlgui::SettingIdx::kUISettingPanelShow)]) {
            const auto f = static_cast<bool>(ui_base_.getProperty(zlgui::SettingIdx::kUISettingPanelShow));
            show_ui_setting_panel_ = f;
        }
    }

    void CurvePanel::repaintCallBack(const double nowT) {
        if (show_ui_setting_panel_) { return; }
        const auto refresh_rate_mul = show_match_panel_.load() ? 2.0 : 1.0;
        if ((nowT - current_t_) * 1000.0 > static_cast<double>(ui_base_.getRefreshRateMS()) * refresh_rate_mul) {
            button_panel_.updateAttach();
            auto is_current_dragger_moved = false;
            for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
                const auto f = button_panel_.updateDragger(i, single_panels_[i]->getButtonPos());
                if (i == previous_band_idx_) is_current_dragger_moved = f;
            }
            if (previous_band_idx_ != band_idx_.load()) {
                if (previous_band_idx_ < zlstate::kBandNUM) {
                    button_panel_.updateLinkButton(previous_band_idx_);
                }
                previous_band_idx_ = band_idx_.load();
                button_panel_.updatePopup(previous_band_idx_);
                button_panel_.updateLinkButton(previous_band_idx_);
            } else {
                button_panel_.updateOtherDraggers(previous_band_idx_,
                                                single_panels_[previous_band_idx_]->getTargetButtonPos());
                button_panel_.updatePopup(previous_band_idx_, is_current_dragger_moved);
                button_panel_.updateLinkButton(previous_band_idx_);
            }

            conflict_panel_.updateGradient();
            loudness_display_.checkVisible();
            solo_panel_.checkVisible();
            for (const auto &panel: single_panels_) {
                panel->updateVisible();
            }
            single_panels_[band_idx_.load()]->toFront(false);
            for (const auto &panel: side_panels_) {
                panel->updateDragger();
            }
            if (show_match_panel_.load()) {
                match_panel_.updateDraggers();
            }
            if (!to_notify_) {
                to_notify_ = true;
                repaint();
                current_t_ = nowT;
            }
        }
    }

    void CurvePanel::run() {
        juce::ScopedNoDenormals no_denormals;
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            const auto factor = physical_pixel_scale_factor_.load();
            const auto &analyzer = controller_ref_.getAnalyzer();
            if (analyzer.getPreON() || analyzer.getPostON() || analyzer.getSideON()) {
                fft_panel_.updatePaths(factor);
            }
            for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
                if (single_panels_[i]->checkRepaint()) {
                    single_panels_[i]->run(factor);
                }
            }
            sum_panel_.run(factor);
            if (show_match_panel_.load()) {
                match_panel_.updatePaths();
            }
        }
    }
}
