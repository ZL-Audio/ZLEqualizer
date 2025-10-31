// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "response_panel.hpp"

namespace zlpanel {
    ResponsePanel::ResponsePanel(PluginProcessor& p,
                                 zlgui::UIBase& base,
                                 const multilingual::TooltipHelper& tooltip_helper) :
        Thread("response"),
        p_ref_(p), base_(base),
        gain_scale_(*p.parameters_.getRawParameterValue(zlp::PGainScale::kID)),
        single_panel_(p, base, message_not_off_indices_),
        sum_panel_(p, base),
        scale_panel_(p, base, tooltip_helper),
        right_click_panel_(p, base, tooltip_helper),
        dragger_panel_(p, base, tooltip_helper, right_click_panel_),
        solo_panel_(p, base),
        eq_max_db_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PEQMaxDB::kID)) {
        juce::ignoreUnused(base_, tooltip_helper);
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto band_str = std::to_string(band);
            for (const auto& ID : kIDs) {
                const auto band_ID = ID + band_str;
                p_ref_.parameters_.addParameterListener(band_ID, this);
                parameterChanged(band_ID,
                                 p_ref_.parameters_.getRawParameterValue(band_ID)->load(std::memory_order::relaxed));
            }
        }
        p_ref_.parameters_.addParameterListener(zlp::PGainScale::kID, this);

        xs_.resize(kNumPoints);
        ws_.resize(kNumPoints);
        for (size_t i = 0; i < zlp::kBandNum; ++i) {
            base_mags_[i].resize(kNumPoints);
            target_mags_[i].resize(kNumPoints);
            dynamic_mags_[i].resize(kNumPoints);
        }
        for (size_t i = 0; i < 5; ++i) {
            sum_mags_[i].resize(kNumPoints);
            on_lr_indices_[i].reserve(zlp::kBandNum);
        }
        for (auto& f : ideal_) {
            f.prepare(480000.0);
        }
        for (auto& f : side_ideal_) {
            f.prepare(480000.0);
        }
        addAndMakeVisible(single_panel_);
        addAndMakeVisible(sum_panel_);
        scale_panel_.setBufferedToImage(true);
        addAndMakeVisible(scale_panel_);
        addAndMakeVisible(dragger_panel_);
        dragger_panel_.addChildComponent(solo_panel_);
        solo_panel_.setAlwaysOnTop(true);
        dragger_panel_.getFloatPopPanel().setAlwaysOnTop(true);
        dragger_panel_.getFloatPopPanel().toFront(true);
        right_click_panel_.setBufferedToImage(true);
        addChildComponent(right_click_panel_);
        setInterceptsMouseClicks(false, true);
    }

    ResponsePanel::~ResponsePanel() {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto band_str = std::to_string(band);
            for (const auto& ID : kIDs) {
                p_ref_.parameters_.removeParameterListener(ID + band_str, this);
            }
        }
        p_ref_.parameters_.removeParameterListener(zlp::PGainScale::kID, this);
    }

    void ResponsePanel::paint(juce::Graphics& g) {
        const std::unique_lock<std::mutex> lock{paint_mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        const auto should_alpha = static_cast<float>(base_.getPanelProperty(zlgui::kCurveShouldTransparent)) > .5f;
        if (should_alpha) {
            g.beginTransparencyLayer(.5f);
        }
        single_panel_.paintDifferentStereo(g);
        sum_panel_.paintDifferentStereo(g);
        single_panel_.paintSameStereo(g);
        sum_panel_.paintSameStereo(g);
        if (should_alpha) {
            g.endTransparencyLayer();
        }
    }

    void ResponsePanel::resized() {
        const auto bound = getLocalBounds();
        const auto font_size = base_.getFontSize();
        const auto bottom_height = getBottomAreaHeight(font_size);
        single_panel_.setBounds(bound);
        sum_panel_.setBounds(bound);
        dragger_panel_.setBounds(bound);
        solo_panel_.setBounds(bound);
        scale_panel_.setBounds(bound.withLeft(bound.getWidth() - scale_panel_.getIdealWidth()));
        right_click_panel_.setBounds(0, 0, right_click_panel_.getIdealWidth(), right_click_panel_.getIdealHeight());
        side_y_ = static_cast<float>(bound.getHeight() - bottom_height) - font_size * kDraggerScale * .5f;
        width_.store(static_cast<float>(bound.getWidth()), std::memory_order::relaxed);
        height_.store(static_cast<float>(bound.getHeight()), std::memory_order::relaxed);
        to_update_bound_.store(true, std::memory_order::release);
    }

    void ResponsePanel::repaintCallBack() {
        updateDraggerPositions();
        updateDrawingParas();
        dragger_panel_.repaintCallBack();
    }

    void ResponsePanel::updateDraggerPositions() {
        updateSoloPosition();
        if (!message_to_update_draggers_total_.exchange(false, std::memory_order::acquire)) {
            return;
        }
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (message_to_update_draggers_[band].exchange(false, std::memory_order::acquire)) {
                dragger_panel_.updateFilterType(band, empty_[band].getFilterType());
                dragger_panel_.getDragger(band).updateButton(
                {points_[band][0].load(std::memory_order::relaxed),
                 points_[band][4].load(std::memory_order::relaxed)});
            }
        }
        updateFloatingPosition();
        updateTargetPosition();
        updateSidePosition();
    }

    void ResponsePanel::updateSoloPosition() {
        if (!solo_panel_.isVisible()) {
            return;
        }
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            if (solo_panel_.isSoloSide()) {
                solo_panel_.updateX(side_points_[band][1].load(std::memory_order::relaxed),
                                    side_points_[band][2].load(std::memory_order::relaxed));
            } else {
                solo_panel_.updateX(points_[band][1].load(std::memory_order::relaxed),
                                    points_[band][2].load(std::memory_order::relaxed));
            }
        }
    }

    void ResponsePanel::updateTargetPosition() {
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            dragger_panel_.getTargetDragger().updateButton(
            {points_[band][0].load(std::memory_order::relaxed),
             points_[band][5].load(std::memory_order::relaxed)});
        }
    }

    void ResponsePanel::updateSidePosition() {
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            dragger_panel_.getSideDragger().updateButton(
            {side_points_[band][0].load(std::memory_order::relaxed),
             side_y_});
        }
    }

    void ResponsePanel::updateFloatingPosition() {
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            dragger_panel_.getFloatPopPanel().updatePosition(
            {points_[band][0].load(std::memory_order::relaxed),
             points_[band][4].load(std::memory_order::relaxed)});
        }
    }

    void ResponsePanel::updateDrawingParas() {
        if (!message_to_update_panels_.exchange(false, std::memory_order::acquire)) {
            return;
        }
        message_not_off_indices_.clear();
        const auto selected_band = base_.getSelectedBand();
        const auto selected_lr_mode = selected_band < zlp::kBandNum
            ? lr_modes_[selected_band].load(std::memory_order::relaxed)
            : 0;
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto filter_status = filter_status_[band].load(std::memory_order::relaxed);
            if (filter_status != zlp::FilterStatus::kOff) {
                message_not_off_indices_.emplace_back(band);
                const auto dynamic_on = dynamic_ons_[band].load(std::memory_order::relaxed);
                const auto lr_mode = lr_modes_[band].load(std::memory_order::relaxed);
                const auto is_same_stereo = selected_band < zlp::kBandNum ? lr_mode == selected_lr_mode : true;
                single_panel_.updateDrawingParas(band, filter_status, dynamic_on, is_same_stereo);
                dragger_panel_.updateDrawingParas(band, filter_status, dynamic_on, is_same_stereo, lr_mode);
                dragger_panel_.getDragger(band).updateButton(
                {points_[band][0].load(std::memory_order::relaxed),
                 points_[band][4].load(std::memory_order::relaxed)});
            } else {
                single_panel_.updateDrawingParas(band, zlp::FilterStatus::kOff, false, false);
                dragger_panel_.updateDrawingParas(band, zlp::FilterStatus::kOff, false, false, 0);
                dragger_panel_.getDragger(band).updateButton({-1000.f, -1000.f});
            }
        }
        updateFloatingPosition();
        updateTargetPosition();
        updateSidePosition();
        for (int lr = 0; lr < 5; ++lr) {
            if (selected_band < zlp::kBandNum) {
                sum_panel_.updateDrawingParas(lr, lr == selected_lr_mode);
            } else {
                sum_panel_.updateDrawingParas(lr, true);
            }
        }
    }

    void ResponsePanel::repaintCallBackSlow() {
        scale_panel_.repaintCallBackSlow();
        dragger_panel_.repaintCallBackSlow();
    }

    void ResponsePanel::updateBand() {
        message_to_update_panels_.store(true, std::memory_order::relaxed);
        dragger_panel_.updateBand();
        solo_panel_.updateBand();
        updateFloatingPosition();
        updateTargetPosition();
        updateSidePosition();
    }

    void ResponsePanel::updateSampleRate(const double sample_rate) {
        sample_rate_.store(sample_rate, std::memory_order::relaxed);
        dragger_panel_.updateSampleRate(sample_rate);
    }

    void ResponsePanel::run() {
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            if (threadShouldExit()) {
                break;
            }
            updateCurveParas();
            if (threadShouldExit()) {
                break;
            }
            if (!updateCurveMags()) {
                break;
            }
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                single_panel_.run(band, c_filter_status_[band],
                                  to_update_base_y_flags_[band],
                                  to_update_target_y_flags_[band],
                                  xs_, c_k_, c_b_,
                                  base_mags_[band], target_mags_[band],
                                  points_[band][0].load(std::memory_order::relaxed),
                                  points_[band][3].load(std::memory_order::relaxed),
                                  points_[band][4].load(std::memory_order::relaxed),
                                  to_update_side_y_flags_[band],
                                  side_points_[band][1].load(std::memory_order::relaxed),
                                  side_points_[band][2].load(std::memory_order::relaxed));
                if (threadShouldExit()) {
                    break;
                }
            }
            {
                std::lock_guard<std::mutex> lock{paint_mutex_};
                single_panel_.runUpdate(to_update_base_y_flags_, to_update_target_y_flags_, to_update_side_y_flags_);
            }
            if (threadShouldExit()) {
                break;
            }
            for (size_t lr = 0; lr < 5; ++lr) {
                sum_panel_.run(lr, to_update_lr_flags_[lr], is_lr_not_off_flags_[lr],
                               on_lr_indices_[lr],
                               xs_, c_k_, c_b_,
                               dynamic_mags_);
                if (threadShouldExit()) {
                    break;
                }
            }
            {
                std::lock_guard<std::mutex> lock{paint_mutex_};
                sum_panel_.runUpdate(to_update_lr_flags_);
            }
            if (threadShouldExit()) {
                break;
            }
        }
    }

    void ResponsePanel::parameterChanged(const juce::String& parameter_ID, const float value) {
        if (parameter_ID.startsWith(zlp::PGainScale::kID)) {
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                empty_[band].setGain(
                    std::clamp(
                        original_base_gains_[band].load(std::memory_order::relaxed) * value / 100.f, -30.f, 30.f));
                to_update_empty_flags_[band].store(true, std::memory_order::release);
                target_gains_[band].store(
                    std::clamp(
                        original_target_gains_[band].load(std::memory_order::relaxed) * value / 100.f, -30.f, 30.f),
                    std::memory_order::relaxed);
                to_update_target_gain_flags_[band].store(true, std::memory_order::release);
            }
            return;
        }
        const auto band = static_cast<size_t>(parameter_ID.getTrailingIntValue());
        if (parameter_ID.startsWith(zlp::PFilterStatus::kID)) {
            filter_status_[band].store(static_cast<zlp::FilterStatus>(std::round(value)), std::memory_order::relaxed);
            to_update_filter_status_.store(true, std::memory_order::release);
        } else if (parameter_ID.startsWith(zlp::PLRMode::kID)) {
            lr_modes_[band].store(static_cast<int>(std::round(value)), std::memory_order::relaxed);
            to_update_lr_modes_.store(true, std::memory_order::release);
        } else if (parameter_ID.startsWith(zlp::PFilterType::kID)) {
            empty_[band].setFilterType(static_cast<zldsp::filter::FilterType>(std::round(value)));
            to_update_empty_flags_[band].store(true, std::memory_order::release);
        } else if (parameter_ID.startsWith(zlp::POrder::kID)) {
            empty_[band].setOrder(zlp::POrder::kOrderArray[static_cast<size_t>(std::round(value))]);
            to_update_empty_flags_[band].store(true, std::memory_order::release);
        } else if (parameter_ID.startsWith(zlp::PFreq::kID)) {
            empty_[band].setFreq(value);
            to_update_empty_flags_[band].store(true, std::memory_order::release);
        } else if (parameter_ID.startsWith(zlp::PGain::kID)) {
            original_base_gains_[band].store(value, std::memory_order::relaxed);
            empty_[band].setGain(
                std::clamp(value * gain_scale_.load(std::memory_order::relaxed) / 100.f, -30.f, 30.f));
            to_update_empty_flags_[band].store(true, std::memory_order::release);
        } else if (parameter_ID.startsWith(zlp::PQ::kID)) {
            empty_[band].setQ(value);
            to_update_empty_flags_[band].store(true, std::memory_order::release);
        } else if (parameter_ID.startsWith(zlp::PDynamicON::kID)) {
            dynamic_ons_[band].store(value > .5f, std::memory_order::relaxed);
            to_update_dynamic_ons_.store(true, std::memory_order::release);
        } else if (parameter_ID.startsWith(zlp::PTargetGain::kID)) {
            original_target_gains_[band].store(value, std::memory_order::relaxed);
            target_gains_[band].store(
                std::clamp(value * gain_scale_.load(std::memory_order::relaxed) / 100.f, -30.f, 30.f),
                std::memory_order::relaxed);
            to_update_target_gain_flags_[band].store(true, std::memory_order::release);
        } else if (parameter_ID.startsWith(zlp::PSideFilterType::kID)) {
            if (value < .5f) {
                side_empty_[band].setFilterType(zldsp::filter::kBandPass);
            } else if (value < 1.5f) {
                side_empty_[band].setFilterType(zldsp::filter::kLowPass);
            } else {
                side_empty_[band].setFilterType(zldsp::filter::kHighPass);
            }
            to_update_side_empty_flags_[band].store(true, std::memory_order::release);
        } else if (parameter_ID.startsWith(zlp::PSideOrder::kID)) {
            side_empty_[band].setOrder(zlp::POrder::kOrderArray[static_cast<size_t>(std::round(value))]);
            to_update_side_empty_flags_[band].store(true, std::memory_order::release);
        } else if (parameter_ID.startsWith(zlp::PSideFreq::kID)) {
            side_empty_[band].setFreq(value);
            to_update_side_empty_flags_[band].store(true, std::memory_order::release);
        } else if (parameter_ID.startsWith(zlp::PSideQ::kID)) {
            side_empty_[band].setQ(value);
            to_update_side_empty_flags_[band].store(true, std::memory_order::release);
        }
    }

    void ResponsePanel::updateCurveParas() {
        // update filter status
        if (to_update_filter_status_.exchange(false, std::memory_order::acquire)) {
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                const auto new_filter_status = filter_status_[band].load(std::memory_order::relaxed);
                if (c_filter_status_[band] != new_filter_status) {
                    c_filter_status_[band] = new_filter_status;
                    to_update_base_y_flags_[band] = true;
                    to_update_lr_flags_[static_cast<size_t>(c_lr_modes_[band])] = true;
                }
            }
            to_update_lr_modes_.store(true, std::memory_order::release);
        }
        // update dynamic ons
        if (to_update_dynamic_ons_.exchange(false, std::memory_order::acquire)) {
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                const auto dynamic_on = dynamic_ons_[band].load(std::memory_order::relaxed);
                if (c_dynamic_ons_[band] != dynamic_on) {
                    c_dynamic_ons_[band] = dynamic_ons_[band].load(std::memory_order::relaxed);
                    to_update_lr_flags_[static_cast<size_t>(c_lr_modes_[band])] = true;
                    if (!dynamic_on) {
                        dynamic_mags_[band] = base_mags_[band];
                    }
                }
            }
            message_to_update_panels_.store(true, std::memory_order::release);
        }
        // update lr modes for summing
        if (to_update_lr_modes_.exchange(false, std::memory_order::acquire)) {
            for (auto& indices : on_lr_indices_) {
                indices.clear();
            }
            std::fill(is_lr_not_off_flags_.begin(), is_lr_not_off_flags_.end(), false);
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                const auto lr_mode = lr_modes_[band].load(std::memory_order::relaxed);
                if (lr_mode != c_lr_modes_[band]) {
                    to_update_lr_flags_[static_cast<size_t>(c_lr_modes_[band])] = true;
                    to_update_lr_flags_[static_cast<size_t>(lr_mode)] = true;
                    c_lr_modes_[band] = lr_mode;
                }
                if (c_filter_status_[band] != zlp::FilterStatus::kOff) {
                    if (c_filter_status_[band] == zlp::FilterStatus::kOn) {
                        on_lr_indices_[static_cast<size_t>(lr_mode)].emplace_back(band);
                    }
                    is_lr_not_off_flags_[static_cast<size_t>(lr_mode)] = true;
                }
            }
            message_to_update_panels_.store(true, std::memory_order::release);
        }
        // update sample rate
        if (const auto sample_rate = sample_rate_.load(std::memory_order::relaxed);
            std::abs(sample_rate - c_sample_rate_) > 1.0) {
            c_sample_rate_ = sample_rate;
            c_slider_max_ = freq_helper::getSliderMax(sample_rate);
            if (sample_rate < 40000.0) {
                return;
            }
            fft_max_ = freq_helper::getFFTMax(sample_rate);
            const auto max_log_value = std::log(fft_max_ * 0.1) / static_cast<double>(
                1.f - kFontSizeOverWidth * kDraggerScale);
            const auto interval_log_value = max_log_value / static_cast<double>(kNumPoints - 1);
            for (size_t i = 0; i < kNumPoints; ++i) {
                ws_[i] = static_cast<float>(std::exp(interval_log_value * static_cast<double>(i)) * kFreqScaleConst);
            }
            std::fill(to_update_base_y_flags_.begin(), to_update_base_y_flags_.end(), true);
        }
        // update width & xs
        if (to_update_bound_.exchange(false, std::memory_order::acquire)) {
            c_width_ = width_.load(std::memory_order::relaxed);
            c_height_ = height_.load(std::memory_order::relaxed);
            if (c_width_ < 1.f || c_height_ < 1.f) {
                return;
            }
            const auto interval_x_value = c_width_ / static_cast<float>(kNumPoints - 1);
            for (size_t i = 0; i < kNumPoints; ++i) {
                xs_[i] = static_cast<float>(i) * interval_x_value;
            }
            c_eq_max_db_idx_ = -1.f;
        }
        // update maximum db
        if (const auto eq_max_db_idx = eq_max_db_idx_ref_.load(std::memory_order::relaxed);
            std::abs(eq_max_db_idx - c_eq_max_db_idx_) > .1f) {
            c_eq_max_db_idx_ = eq_max_db_idx;
            const auto z = zlstate::PEQMaxDB::kDBs[static_cast<size_t>(std::round(eq_max_db_idx))];
            const auto h = c_height_ - static_cast<float>(getBottomAreaHeight(c_width_ * kFontSizeOverWidth));
            const auto padding = c_width_ * kFontSizeOverWidth * kDraggerScale;
            const auto h1 = h * .5f;
            const auto h2 = h - padding;
            c_k_ = 10.f * (h1 - h2) / z;
            c_b_ = h1;
            std::fill(to_update_base_y_flags_.begin(), to_update_base_y_flags_.end(), true);
        }
        // update db update flags
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            to_update_base_y_flags_[band] = to_update_base_y_flags_[band]
                || to_update_empty_flags_[band].exchange(false, std::memory_order::acquire);
            to_update_target_y_flags_[band] = to_update_base_y_flags_[band]
                || to_update_target_gain_flags_[band].exchange(false, std::memory_order::acquire);
            to_update_side_y_flags_[band] = to_update_base_y_flags_[band]
                || to_update_side_empty_flags_[band].exchange(false, std::memory_order::acquire);
            const auto lr = c_lr_modes_[band];
            to_update_lr_flags_[static_cast<size_t>(lr)] = to_update_lr_flags_[static_cast<size_t>(lr)]
                || to_update_base_y_flags_[band] || c_dynamic_ons_[band];
        }
    }

    bool ResponsePanel::updateCurveMags() {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (c_filter_status_[band] != zlp::FilterStatus::kOff) {
                if (to_update_base_y_flags_[band]) {
                    auto para = empty_[band].getParas();
                    para.freq = std::min(para.freq, c_slider_max_);
                    ideal_[band].forceUpdate(para);
                    ideal_[band].updateMagnitudeSquare(ws_, base_mags_[band]);
                    base_mags_[band] = kfr::log10(kfr::max(base_mags_[band], 1e-24f));
                    if (!c_dynamic_ons_[band]) {
                        dynamic_mags_[band] = base_mags_[band];
                    }
                    const auto center_w = para.freq * (2.0 * std::numbers::pi / 480000.0);
                    const float center_square_magnitude = std::log10(std::max(
                        ideal_[band].getCenterMagnitudeSquare(static_cast<float>(center_w)), 1e-24f));
                    const auto [left_x, center_x, right_x] = getLeftCenterRightX(para);

                    points_[band][0].store(static_cast<float>(center_x), std::memory_order::relaxed);
                    points_[band][1].store(static_cast<float>(left_x), std::memory_order::relaxed);
                    points_[band][2].store(static_cast<float>(right_x), std::memory_order::relaxed);
                    points_[band][3].store(c_k_ * center_square_magnitude + c_b_, std::memory_order::relaxed);
                    para.gain = original_base_gains_[band].load(std::memory_order::relaxed);
                    points_[band][4].store(c_k_ * getButtonMag(para) + c_b_, std::memory_order::relaxed);

                    if (threadShouldExit()) {
                        return false;
                    }
                    message_to_update_draggers_[band].store(true, std::memory_order::release);
                    message_to_update_draggers_total_.store(true, std::memory_order::release);
                }
                if (to_update_target_y_flags_[band]) {
                    const auto target_gain = target_gains_[band].load(std::memory_order::relaxed);
                    ideal_[band].setGain(target_gain);
                    ideal_[band].updateCoeffs();
                    ideal_[band].updateMagnitudeSquare(ws_, target_mags_[band]);
                    target_mags_[band] = kfr::log10(kfr::max(target_mags_[band], 1e-24f));
                    auto para = ideal_[band].getParas();
                    para.gain = original_target_gains_[band].load(std::memory_order::relaxed);
                    points_[band][5].store(c_k_ * getButtonMag(para) + c_b_, std::memory_order::relaxed);

                    if (threadShouldExit()) {
                        return false;
                    }
                    message_to_update_target_dragger_.store(true, std::memory_order::release);
                    message_to_update_draggers_total_.store(true, std::memory_order::release);
                }
                if (to_update_side_y_flags_[band]) {
                    auto para = side_empty_[band].getParas();
                    para.freq = std::min(para.freq, c_slider_max_);
                    const auto [left_x, center_x, right_x] = getLeftCenterRightX(para);
                    side_points_[band][0].store(center_x, std::memory_order::relaxed);
                    if (para.filter_type == zldsp::filter::kLowPass) {
                        side_points_[band][1].store(0.f, std::memory_order::relaxed);
                        side_points_[band][2].store(center_x, std::memory_order::relaxed);
                    } else if (para.filter_type == zldsp::filter::kHighPass) {
                        side_points_[band][1].store(center_x, std::memory_order::relaxed);
                        side_points_[band][2].store(c_width_, std::memory_order::relaxed);
                    } else {
                        side_points_[band][1].store(left_x, std::memory_order::relaxed);
                        side_points_[band][2].store(right_x, std::memory_order::relaxed);
                    }
                    message_to_update_side_dragger_.store(true, std::memory_order::release);
                    message_to_update_draggers_total_.store(true, std::memory_order::release);
                }
                if (c_dynamic_ons_[band]) {
                    ideal_[band].setGain(p_ref_.getController().getCurrentGain(band));
                    ideal_[band].updateCoeffs();
                    ideal_[band].updateMagnitudeSquare(ws_, dynamic_mags_[band]);
                    dynamic_mags_[band] = kfr::log10(kfr::max(dynamic_mags_[band], 1e-24f));
                    if (threadShouldExit()) {
                        return false;
                    }
                }
            } else {
                points_[band][4].store(-10000.f, std::memory_order::relaxed);
            }
        }
        return true;
    }

    float ResponsePanel::getButtonMag(const zldsp::filter::FilterParameters& para) {
        if (para.filter_type == zldsp::filter::kPeak) {
            return static_cast<float>(0.1 * para.gain);
        } else if (para.filter_type == zldsp::filter::kLowShelf
            || para.filter_type == zldsp::filter::kHighShelf
            || para.filter_type == zldsp::filter::kTiltShelf) {
            return static_cast<float>(0.05 * para.gain);
        } else {
            return 0.f;
        }
    }

    std::tuple<float, float, float> ResponsePanel::getLeftCenterRightX(
        const zldsp::filter::FilterParameters& para) const {
        const auto freq_to_x_scale = 1.0 / std::log(
            fft_max_ * 0.1) * static_cast<double>(c_width_) * static_cast<double>(
            1.f - kFontSizeOverWidth * kDraggerScale);
        const auto center_x = std::log(para.freq / 10.0) * freq_to_x_scale;
        const auto bandwidth = para.freq / para.q;
        switch (para.filter_type) {
        case zldsp::filter::kPeak:
        case zldsp::filter::kBandPass:
        case zldsp::filter::kNotch:
        case zldsp::filter::kTiltShelf:
        case zldsp::filter::kBandShelf:
        default: {
            const auto left_f = 0.5 * bandwidth * (std::sqrt(4.0 * para.q * para.q + 1.0) - 1.0);
            const auto left_x = std::log(left_f / 10.0) * freq_to_x_scale;
            const auto right_f = left_f + bandwidth;
            const auto right_x = std::log(right_f / 10.0) * freq_to_x_scale;
            return std::make_tuple(static_cast<float>(left_x),
                                   static_cast<float>(center_x),
                                   static_cast<float>(right_x));
        }
        case zldsp::filter::kLowShelf:
        case zldsp::filter::kHighPass: {
            return std::make_tuple(0.f, static_cast<float>(center_x), static_cast<float>(center_x));
        }
        case zldsp::filter::kHighShelf:
        case zldsp::filter::kLowPass: {
            return std::make_tuple(static_cast<float>(center_x), static_cast<float>(center_x), c_width_);
        }
        }
    }
}
