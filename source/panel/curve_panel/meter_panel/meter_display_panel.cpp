// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "meter_display_panel.hpp"
#include "../../../dsp/chore/decibels.hpp"

namespace zlpanel {
    MeterDisplayPanel::MeterDisplayPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base),
        meter_top_panel_(base),
        fft_top_db_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTTopDB::kID)),
        fft_min_db_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTMinDB::kID)) {
        meter_top_panel_.setBufferedToImage(true);
        addAndMakeVisible(meter_top_panel_);
    }

    void MeterDisplayPanel::paint(juce::Graphics& g) {
        bounds_.pull();
        const auto& bounds{bounds_.getReader()};
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPreColour).withAlpha(.25f));
        for (const auto& bound : bounds.pre) {
            g.fillRect(bound);
        }
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPostColour).withAlpha(1.f));
        for (const auto& bound : bounds.out) {
            g.fillRect(bound);
        }
        for (const auto& bound : bounds.arrow) {
            g.fillRect(bound);
        }
    }

    void MeterDisplayPanel::resized() {
        auto bound = getLocalBounds();
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        meter_top_panel_.setBounds(bound.removeFromTop(getTopPanelHeight(font_size) - padding / 2));
        bound.removeFromTop(padding / 2);
        pending_bound_.store(bound.toFloat());
        lookAndFeelChanged();
    }

    void MeterDisplayPanel::repaintCallBackSlow() {
        meter_top_panel_.updateValue(out_peak_.load(std::memory_order::relaxed));
    }

    void MeterDisplayPanel::pull(const zldsp::container::FIFORange range,
                                 const std::vector<std::vector<float>>& pre_fifo,
                                 const std::vector<std::vector<float>>& out_fifo) {
        for (size_t chan = 0; chan < 2; ++chan) {
            if (chan < pre_fifo.size()) {
                pre_peaks_[chan] = std::max(pre_peaks_[chan],
                                            zldsp::analyzer::MagAnalyzerOps::calculatePeak(range, pre_fifo[chan]));
            }
            if (chan < out_fifo.size()) {
                out_peaks_[chan] = std::max(out_peaks_[chan],
                                            zldsp::analyzer::MagAnalyzerOps::calculatePeak(range, out_fifo[chan]));
            }
        }
    }

    void MeterDisplayPanel::run(const double time_stamp) {
        const auto delta_time = previous_time_ > 0.0
            ? std::clamp(time_stamp - previous_time_, 0.001, 1.0)
            : 1.0 / 30.0;
        previous_time_ = time_stamp;
        if (reset_peaks_.exchange(false, std::memory_order::relaxed)) {
            out_peak_.store(-240.f, std::memory_order::relaxed);
        }

        const auto top_db = zlstate::PFFTTopDB::kDBs[static_cast<size_t>(std::round(
            fft_top_db_idx_ref_.load(std::memory_order::relaxed)))];
        const auto range_db = zlstate::PFFTMinDB::kDBs[static_cast<size_t>(std::round(
            fft_min_db_idx_ref_.load(std::memory_order::relaxed)))];
        const auto bound = pending_bound_.load();
        const auto font_size = pending_font_size_.load(std::memory_order::relaxed);
        const auto thickness = font_size * .2f;
        const auto padding = font_size * kDraggerScale;
        const auto scale_top = bound.getY() + padding;
        const auto scale_height = std::max(1.f, bound.getHeight()
                                           - static_cast<float>(getBottomAreaHeight(font_size)) - 2.f * padding);
        const auto meter_width = bound.getWidth() / 2.5f;
        const auto meter_padding = meter_width * .5f;
        auto& bounds{bounds_.getWriter()};

        // update independent meter decays, then constrain their displayed gap
        const auto meter_delta_time = static_cast<float>(delta_time);
        const auto meter_decay = meter_delta_time * kMeterDecayPerSecond;
        const auto decay_acceleration = 1.f + 3.f * meter_delta_time;
        for (size_t chan = 0; chan < 2; ++chan) {
            const auto current_pre = zldsp::chore::gainToDecibels(pre_peaks_[chan]);
            const auto current_out = zldsp::chore::gainToDecibels(out_peaks_[chan]);
            pre_peaks_[chan] = 0.f;
            out_peaks_[chan] = 0.f;
            out_peak_.store(std::max(current_out, out_peak_.load(std::memory_order::relaxed)),
                            std::memory_order::relaxed);
            // advance from the last displayed gap
            const auto previous_gap = static_cast<double>(previous_out_[chan]) - previous_pre_[chan];
            const auto target_gap = static_cast<double>(current_out) - current_pre;
            if (std::abs(target_gap - target_gap_db_[chan]) > 1e-5) {
                target_gap_db_[chan] = target_gap;
                gap_remaining_seconds_[chan] = kMeterGapConvergenceSeconds;
            }
            const auto remaining_seconds = gap_remaining_seconds_[chan];
            auto gap = remaining_seconds > delta_time
                ? std::lerp(previous_gap, target_gap, delta_time / remaining_seconds)
                : target_gap;
            gap_remaining_seconds_[chan] = std::max(0.0, remaining_seconds - delta_time);

            const auto pre_attack = current_pre > previous_pre_[chan];
            const auto out_attack = current_out > previous_out_[chan];
            previous_pre_[chan] = std::max(previous_pre_[chan] - pre_decay_mul_[chan] * meter_decay, current_pre);
            previous_out_[chan] = std::max(previous_out_[chan] - out_decay_mul_[chan] * meter_decay, current_out);
            pre_decay_mul_[chan] = pre_attack
                ? 1.f
                : std::min(pre_decay_mul_[chan] * decay_acceleration, 10.f);
            out_decay_mul_[chan] = out_attack
                ? 1.f
                : std::min(out_decay_mul_[chan] * decay_acceleration, 10.f);

            gap = std::clamp(gap, static_cast<double>(current_out) - previous_pre_[chan],
                             static_cast<double>(previous_out_[chan]) - current_pre);
            if (target_gap > 0.0) {
                gap = std::max(gap, 0.0);
            } else if (target_gap < 0.0) {
                gap = std::min(gap, 0.0);
            }
            // keep the corrected positions as the next frame's decay state
            if (static_cast<double>(previous_out_[chan]) - previous_pre_[chan] > gap) {
                previous_out_[chan] = std::clamp(static_cast<float>(previous_pre_[chan] + gap),
                                                 current_out, previous_out_[chan]);
            } else {
                previous_pre_[chan] = std::clamp(static_cast<float>(previous_out_[chan] - gap),
                                                 current_pre, previous_pre_[chan]);
            }

            const auto pre_y = std::clamp((previous_pre_[chan] - top_db) / range_db * scale_height + scale_top,
                                          bound.getY(), bound.getBottom());
            const auto out_y = std::clamp((previous_out_[chan] - top_db) / range_db * scale_height + scale_top,
                                          bound.getY(), bound.getBottom());
            const auto x = bound.getX() + static_cast<float>(chan) * (meter_width + meter_padding);
            bounds.pre[chan] = {x, pre_y, meter_width, bound.getBottom() - pre_y};
            bounds.out[chan] = {x, out_y - thickness * .5f, meter_width, thickness};
            bounds.arrow[chan] = {x + meter_width * .5f - thickness * .5f,
                                  std::min(pre_y, out_y), thickness, std::abs(out_y - pre_y)};
        }
        bounds_.publish();
    }

    void MeterDisplayPanel::reset() {
        pre_peaks_.fill(0.f);
        out_peaks_.fill(0.f);
        previous_pre_.fill(-240.f);
        previous_out_.fill(-240.f);
        pre_decay_mul_.fill(1.f);
        out_decay_mul_.fill(1.f);
        target_gap_db_.fill(0.0);
        gap_remaining_seconds_.fill(0.0);
        previous_time_ = 0.0;
        out_peak_.store(-240.f, std::memory_order::relaxed);
    }

    void MeterDisplayPanel::mouseDoubleClick(const juce::MouseEvent&) {
        reset_peaks_.store(true, std::memory_order::relaxed);
    }

    void MeterDisplayPanel::lookAndFeelChanged() {
        pending_font_size_.store(base_.getFontSize(), std::memory_order::relaxed);
    }
}
