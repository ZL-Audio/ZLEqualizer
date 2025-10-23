// Copyright (C) 2025 - zsliu98
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
                             const multilingual::TooltipHelper& tooltip_helper) :
        p_ref_(p), base_(base) {
        juce::ignoreUnused(tooltip_helper);
    }

    MouseEventPanel::~MouseEventPanel() {
        stopTimer(0);
        stopTimer(1);
    }

    void MouseEventPanel::mouseEnter(const juce::MouseEvent&) {
        startTimer(1, 2000);
    }

    void MouseEventPanel::mouseMove(const juce::MouseEvent&) {
        stopTimer(1);
        for (size_t i = 0; i < 3; ++i) {
            p_ref_.getController().getFFTAnalyzer().setFrozen(i, false);
        }
        startTimer(1, 2000);
    }

    void MouseEventPanel::mouseExit(const juce::MouseEvent&) {
        stopTimer(1);
    }

    void MouseEventPanel::mouseDown(const juce::MouseEvent&) {
        startTimer(0, 200);
    }

    void MouseEventPanel::mouseDoubleClick(const juce::MouseEvent& event) {
        stopTimer(0);
        // find an off band
        size_t band_idx = zlp::kBandNum;
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (getValue(p_ref_.parameters_, zlp::PFilterStatus::kID + std::to_string(band)) < .1f) {
                band_idx = band;
                break;
            }
        }
        if (band_idx == zlp::kBandNum) {
            return;
        }

        const auto padding = base_.getFontSize() * kDraggerScale;
        const auto width = static_cast<float>(getLocalBounds().getWidth());
        const auto x_portion = event.position.x / (width - padding);
        const auto freq = std::clamp(std::exp(x_portion * std::log(fft_max_ * 0.1f)) * 10.f, 10.f, slider_max_);

        const auto height = static_cast<float>(getLocalBounds().getHeight() - getBottomAreaHeight(base_.getFontSize()));
        const auto y_portion = (height - 2 * event.position.y) / (height - 2 * padding) ;
        const auto max_db = zlstate::PEQMaxDB::kDBs[static_cast<size_t>(std::round(
            getValue(p_ref_.parameters_NA_,zlstate::PEQMaxDB::kID)))];

        std::array<float, kInitIDs.size()> init_values{};
        init_values[0] = 2.f;
        init_values[5] = 0.f;
        if (previous_band_ < zlp::kBandNum) {
            init_values[2] = getValue(p_ref_.parameters_, zlp::PLRMode::kID + std::to_string(previous_band_));
        } else {
            init_values[2] = 0.f;
        }

        if (event.position.y > height - padding) {
            init_values[1] = static_cast<float>(zldsp::filter::FilterType::kNotch);
        } else if (freq < 20.f && std::abs(y_portion) < .125f) {
            init_values[1] = static_cast<float>(zldsp::filter::FilterType::kHighPass);
        } else if (freq > 10000.f && std::abs(y_portion) < .125f) {
            init_values[1] = static_cast<float>(zldsp::filter::FilterType::kLowPass);
        } else if (freq < 40.f) {
            init_values[1] = static_cast<float>(zldsp::filter::FilterType::kLowShelf);
            init_values[5] = std::clamp(y_portion * 2.f, -.1f, 1.f) * max_db;
        } else if (freq > 6250.f) {
            init_values[1] = static_cast<float>(zldsp::filter::FilterType::kHighShelf);
            init_values[5] = std::clamp(y_portion * 2.f, -.1f, 1.f) * max_db;
        } else {
            init_values[1] = static_cast<float>(zldsp::filter::FilterType::kPeak);
            init_values[5] = std::clamp(y_portion, -1.f, 1.f) * max_db;
        }

        init_values[3] = 1.f;
        init_values[4] = freq;
        init_values[6] = 0.707f;
        init_values[7] = event.mods.isCommandDown() ? 1.f : 0.f;

        for (size_t i = 0; i < kInitIDs.size(); ++i) {
            auto *para = p_ref_.parameters_.getParameter(kInitIDs[i] + std::to_string(band_idx));
            updateValue(para, para->convertTo0to1(init_values[i]));
        }
        band_helper::turnOnOffDynamic(p_ref_, band_idx, init_values[6] > .5f);
        base_.setSelectedBand(band_idx);
    }

    void MouseEventPanel::updateBand() {
        if (base_.getSelectedBand() < zlp::kBandNum) {
            previous_band_ = base_.getSelectedBand();
        }
    }

    void MouseEventPanel::updateSampleRate(const double sample_rate) {
        fft_max_ = static_cast<float>(freq_helper::getFFTMax(sample_rate));
        slider_max_ = static_cast<float>(freq_helper::getSliderMax(sample_rate));
    }

    void MouseEventPanel::timerCallback(const int timer_ID) {
        if (timer_ID == 0) {
            base_.setSelectedBand(zlp::kBandNum);
            stopTimer(0);
        } else if (timer_ID == 1) {
            for (size_t i = 0; i < 3; ++i) {
                p_ref_.getController().getFFTAnalyzer().setFrozen(i, true);
            }
            stopTimer(1);
        }
    }
}
