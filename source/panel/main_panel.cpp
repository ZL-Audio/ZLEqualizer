// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "main_panel.hpp"

namespace zlpanel {
    MainPanel::MainPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p), base_(base),
        tooltip_helper_(
            static_cast<multilingual::TooltipLanguage>(std::round(
                getValue(p_ref_.state_, zlstate::PTooltipLang::kID)))
            ),
        refresh_handler_(zlstate::PTargetRefreshSpeed::kRates[base_.getRefreshRateID()]),
        control_panel_(p, base, tooltip_helper_),
        curve_panel_(p, base, tooltip_helper_),
        tooltip_laf_(base_), tooltip_window_(&curve_panel_) {
        juce::ignoreUnused(base_);

        tooltip_window_.setLookAndFeel(&tooltip_laf_);
        tooltip_window_.setOpaque(false);
        tooltip_window_.setBufferedToImage(true);

        base_.getPanelValueTree().addListener(this);

        startTimerHz(1);

        addAndMakeVisible(curve_panel_);
        addChildComponent(control_panel_);
    }

    MainPanel::~MainPanel() {
        base_.getPanelValueTree().removeListener(this);
        stopTimer();
    }

    void MainPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColour());
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds();

        // set actual width/height
        {
            const auto height = static_cast<float>(bound.getHeight());
            const auto width = static_cast<float>(bound.getWidth());
            if (height < width * 0.47f) {
                bound.setHeight(juce::roundToInt(width * .47f));
            } else if (height > width * 1.f) {
                bound.setWidth(juce::roundToInt(height * 1.f));
            }
        }

        const auto font_size = static_cast<float>(bound.getWidth()) * kFontSizeOverWidth;
        base_.setFontSize(font_size);
        // set control panel bound
        auto control_bound = bound;
        control_bound.removeFromBottom(getBottomPadding(base_.getFontSize()));
        control_bound = control_bound.removeFromBottom(control_panel_.getIdealHeight());
        control_bound = control_bound.withSizeKeepingCentre(control_panel_.getIdealWidth(), control_bound.getHeight());
        control_panel_.setBounds(control_bound);

        bound.removeFromTop(getButtonSize(base_.getFontSize()));
        curve_panel_.setBounds(bound);
    }

    void MainPanel::repaintCallBack(const double time_stamp) {
        if (refresh_handler_.tick(time_stamp)) {
            if (time_stamp - previous_time_stamp_ > 0.1) {
                previous_time_stamp_ = time_stamp;
                repaintCallBackSlow();
            }
            // update selected band
            if (c_band_ != base_.getSelectedBand()) {
                c_band_ = base_.getSelectedBand();
                control_panel_.updateBand();
                curve_panel_.updateBand();
            }
            curve_panel_.repaintCallBack();
            const auto c_refresh_rate = refresh_handler_.getActualRefreshRate();
            if (std::abs(c_refresh_rate - refresh_rate_) > 0.1) {
                refresh_rate_ = c_refresh_rate;
                p_ref_.getController().getFFTAnalyzer().setRefreshRate(static_cast<float>(refresh_rate_));
            }
        }
    }

    void MainPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kUISettingPanel, property)) {
            const auto ui_setting_visibility = static_cast<bool>(
                base_.getPanelProperty(zlgui::PanelSettingIdx::kUISettingPanel));
            juce::ignoreUnused(ui_setting_visibility);
        }
    }

    void MainPanel::timerCallback() {
        if (juce::Process::isForegroundProcess()) {
            if (getCurrentlyFocusedComponent() != this) {
                grabKeyboardFocus();
            }
            stopTimer();
        }
    }

    void MainPanel::repaintCallBackSlow() {
        // update sample rate
        const auto sample_rate = p_ref_.getAtomicSampleRate();
        if (std::abs(sample_rate - c_sample_rate_) > 1.0) {
            c_sample_rate_ = sample_rate;
            curve_panel_.updateSampleRate(sample_rate);
            control_panel_.updateSampleRate(sample_rate);
        }
        // sub slow callbacks
        control_panel_.repaintCallBackSlow();
        curve_panel_.repaintCallBackSlow();
    }

    void MainPanel::startThreads() {
        curve_panel_.startThreads();
    }

    void MainPanel::stopThreads() {
        curve_panel_.stopThreads();
    }
}
