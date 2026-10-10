// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "mouse_event_panel.hpp"

namespace zlpanel {
    MouseEventPanel::MouseEventPanel(PluginProcessor& p,
                                     zlgui::UIBase& base,
                                     const multilingual::TooltipHelper& tooltip_helper,
                                     RightClickPanel& right_click_panel) :
        p_ref_(p), base_(base), right_click_panel_(right_click_panel),
        fft_freeze_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTFreezeON::kID)),
        q_slider_(base), slope_slider_(base) {
        juce::ignoreUnused(tooltip_helper);
    }

    MouseEventPanel::~MouseEventPanel() {
        stopTimer();
        setFFTFreeze(false);
    }

    void MouseEventPanel::mouseEnter(const juce::MouseEvent&) {
        if (q_attachment_) {
            q_attachment_->updateComponent();
        }
    }

    void MouseEventPanel::mouseMove(const juce::MouseEvent& event) {
        if (event.mods.isShiftDown() != is_shift_down_) {
            is_shift_down_ = ! is_shift_down_;
            updateFFTFreeze();
        }
    }

    void MouseEventPanel::mouseDown(const juce::MouseEvent& event) {
        const auto action_type = event.mods.isRightButtonDown()
            ? zlgui::MouseActionType::kRightClick
            : zlgui::MouseActionType::kLeftClick;
        if (base_.isRightClickTriggered(action_type, event.mods)) {
            base_.setSelectedBand(zlp::kBandNum);
            right_click_panel_.setPosition(event.position);
            right_click_panel_.setVisible(true);
            return;
        }

        if (event.mods.isLeftButtonDown()) {
            right_click_panel_.setVisible(false);
            startTimer(200);
        }
    }

    void MouseEventPanel::mouseDrag(const juce::MouseEvent&) {
        stopTimer();
    }

    void MouseEventPanel::visibilityChanged() {
        if (!isShowing()) {
            has_mouse_position_ = false;
            setFFTFreeze(false);
        }
    }

    void MouseEventPanel::mouseDoubleClick(const juce::MouseEvent& event) {
        stopTimer();
        const auto action_type = event.mods.isRightButtonDown()
            ? zlgui::MouseActionType::kRightDoubleClick
            : zlgui::MouseActionType::kLeftDoubleClick;
        if (base_.isRightClickTriggered(action_type, event.mods)) {
            base_.setSelectedBand(zlp::kBandNum);
            right_click_panel_.setPosition(event.position);
            right_click_panel_.setVisible(true);
            return;
        }

        // find an off band
        const size_t band_idx = band_helper::findOffBand(p_ref_);
        if (band_idx == zlp::kBandNum) {
            return;
        }

        const auto padding = base_.getFontSize() * kDraggerScale;
        const auto width = static_cast<float>(getLocalBounds().getWidth());
        const auto x_portion = event.position.x / (width * kFFTSizeOverWidth);
        const auto freq = std::clamp(std::exp(x_portion * std::log(fft_max_ * 0.1f)) * 10.f, 10.f, slider_max_);

        const auto height = static_cast<float>(getLocalBounds().getHeight() - getBottomAreaHeight(base_.getFontSize()));
        const auto y_portion = (height - 2 * event.position.y) / (height - 2 * padding);
        const auto max_db = base_.getCurveDBScale(static_cast<size_t>(std::round(
            getValue(p_ref_.parameters_NA_, zlstate::PEQMaxDB::kID))));

        std::array<float, kInitIDs.size()> init_values{};
        init_values[0] = 2.f;
        init_values[5] = 0.f;
        if (previous_band_ < zlp::kBandNum) {
            init_values[2] = getValue(p_ref_.parameters_, zlp::PLRMode::kID + std::to_string(previous_band_));
        } else {
            init_values[2] = 0.f;
        }

        const auto filter_type = event.position.y > height - padding
            ? zldsp::filter::FilterType::kNotch
            : band_helper::getInitialFilterType(freq, y_portion);
        init_values[1] = static_cast<float>(filter_type);
        if (filter_type == zldsp::filter::FilterType::kLowShelf
            || filter_type == zldsp::filter::FilterType::kHighShelf) {
            init_values[5] = std::clamp(y_portion * 2.f, -1.f, 1.f) * max_db;
        } else if (filter_type == zldsp::filter::FilterType::kPeak) {
            init_values[5] = std::clamp(y_portion, -1.f, 1.f) * max_db;
        }

        init_values[3] = 1.f;
        init_values[4] = freq;
        init_values[6] = 0.707f;
        init_values[7] = event.mods.isCommandDown() ? 1.f : 0.f;

        for (size_t i = 0; i < kInitIDs.size(); ++i) {
            auto* para = p_ref_.parameters_.getParameter(kInitIDs[i] + std::to_string(band_idx));
            updateValue(para, para->convertTo0to1(init_values[i]));
        }
        band_helper::turnOnOffDynamic(p_ref_, band_idx, init_values[7] > .5f, max_db);
        base_.setSelectedBand(band_idx);
    }

    void MouseEventPanel::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
        if (static_cast<int>(std::round(c_ftype_idx_)) == static_cast<int>(zldsp::filter::kFlatGain)) {
            return;
        }
        if (event.mods.isCommandDown()) {
            slope_slider_.mouseWheelMove(event, wheel);
        } else {
            q_slider_.mouseWheelMove(event, wheel);
        }
    }

    void MouseEventPanel::repaintCallbackSlow() {
        if (ftype_idx_ref_) {
            const auto ftype_idx = ftype_idx_ref_->load(std::memory_order::relaxed);
            if (std::abs(ftype_idx - c_ftype_idx_) > .1f) {
                c_ftype_idx_ = ftype_idx;
                updateSlopeAttachment();
            }
        }
        c_fft_freeze_ = fft_freeze_ref_.load(std::memory_order::relaxed) > .5f;
        updateFFTFreeze();
        updater_.updateComponents();
    }

    void MouseEventPanel::updateBand() {
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            previous_band_ = band;
            q_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                q_slider_, p_ref_.parameters_, zlp::PQ::kID + std::to_string(band), updater_);
            q_attachment_->updateComponent();
            ftype_idx_ref_ = p_ref_.parameters_.getRawParameterValue(zlp::PFilterType::kID + std::to_string(band));
        } else {
            q_attachment_.reset();
            ftype_idx_ref_ = nullptr;
        }
        updateSlopeAttachment();
    }

    void MouseEventPanel::updateSampleRate(const double sample_rate) {
        fft_max_ = static_cast<float>(freq_helper::getFFTMax(sample_rate));
        slider_max_ = static_cast<float>(freq_helper::getSliderMax(sample_rate));
    }

    void MouseEventPanel::timerCallback() {
        base_.setSelectedBand(zlp::kBandNum);
        stopTimer();
    }

    void MouseEventPanel::updateSlopeAttachment() {
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            const auto ftype = static_cast<int>(c_ftype_idx_);
            const auto slope_6_disabled = (ftype == static_cast<int>(zldsp::filter::kPeak))
                || (ftype == static_cast<int>(zldsp::filter::kBandPass))
                || (ftype == static_cast<int>(zldsp::filter::kNotch));
            slope_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                slope_slider_, p_ref_.parameters_, zlp::POrder::kID + std::to_string(band),
                juce::NormalisableRange<double>(slope_6_disabled ? 1.0 : 0.0, 6.0, 1.0),
                updater_);
        } else {
            slope_attachment_.reset();
        }
    }

    void MouseEventPanel::updateFFTFreeze() {
        if (!c_fft_freeze_) {
            has_mouse_position_ = false;
            setFFTFreeze(false);
            return;
        }

        const auto source = juce::Desktop::getInstance().getMainMouseSource();
        const auto screen_position = source.getScreenPosition();

        if (!has_mouse_position_ || screen_position != last_mouse_screen_position_) {
            mouse_idle_updates_ = 0;
        } else if (mouse_idle_updates_ < kFFTFreezeIdleUpdates) {
            ++mouse_idle_updates_;
        }
        has_mouse_position_ = true;
        last_mouse_screen_position_ = screen_position;

        setFFTFreeze(is_shift_down_ || mouse_idle_updates_ >= kFFTFreezeIdleUpdates);
    }

    void MouseEventPanel::setFFTFreeze(const bool should_freeze) {
        if (is_fft_frozen_ != should_freeze) {
            is_fft_frozen_ = should_freeze;
            base_.setPanelProperty(zlgui::PanelSettingIdx::kFFTFrozen, should_freeze ? 1.f : 0.f);
            base_.setPanelProperty(zlgui::kCurveShouldTransparent, should_freeze ? 1.f : 0.f);
        }
    }
}
