// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "solo_panel.hpp"

#include "../../../state/state_definitions.hpp"

namespace zlpanel {
    SoloPanel::SoloPanel(juce::AudioProcessorValueTreeState &parameters,
                         juce::AudioProcessorValueTreeState &parameters_NA,
                         zlgui::UIBase &base,
                         zlp::Controller<double> &controller,
                         ButtonPanel &button_panel)
        : parameters_ref_(parameters), parameters_NA_ref_(parameters_NA),
          ui_base_(base),
          soloF(controller.getSoloFilter()),
          controller_ref_(controller), button_panel_ref_(button_panel) {
        juce::ignoreUnused(parameters_ref_, parameters_NA);
        parameters_NA_ref_.addParameterListener(zlstate::selectedBandIdx::ID, this);
        for (size_t i = 0; i < zlp::kBandNUM; ++i) {
            solo_updaters_.emplace_back(std::make_unique<zldsp::chore::ParaUpdater>(
                parameters_ref_, zlp::appendSuffix(zlp::solo::ID, i)));
            side_solo_updaters_.emplace_back(std::make_unique<zldsp::chore::ParaUpdater>(
                parameters_ref_, zlp::appendSuffix(zlp::sideSolo::ID, i)));
        }
        band_idx_.store(static_cast<size_t>(
            parameters_NA_ref_.getRawParameterValue(zlstate::selectedBandIdx::ID)->load()));
        handleAsyncUpdate();
    }

    SoloPanel::~SoloPanel() {
        turnOffSolo();
        parameters_NA_ref_.removeParameterListener(zlstate::selectedBandIdx::ID, this);
    }

    void SoloPanel::paint(juce::Graphics &g) {
        const size_t c_band_idx = band_idx_.load();

        g.setColour(ui_base_.getTextColor().withAlpha(.1f));
        auto bound = getLocalBounds().toFloat();
        if (controller_ref_.getSoloIsSide()) {
            const auto x =  button_panel_ref_.getSideDragger(
                band_idx_.load()).getButton().getBoundsInParent().toFloat().getCentreX();
            if (std::abs(x - current_x_) >= 0.001 || std::abs(soloF.getQ() - solo_q_) >= 0.001) {
                current_x_ = x;
                handleAsyncUpdate();
            }
            const auto bound_width = bound.getWidth();
            const auto left_width = current_x_ - current_bw_ * bound_width;
            const auto right_width = bound_width - current_x_ - current_bw_ * bound_width;
            const auto left_area = bound.removeFromLeft(left_width);
            const auto right_area = bound.removeFromRight(right_width);
            g.fillRect(left_area);
            g.fillRect(right_area);
        } else {
            const auto x = button_panel_ref_.getDragger(c_band_idx
                ).getButton().getBoundsInParent().toFloat().getCentreX();
            if (std::abs(x - current_x_) >= 0.001 || std::abs(soloF.getQ() - solo_q_) >= 0.001) {
                current_x_ = x;
                handleAsyncUpdate();
            }
            const auto &f = controller_ref_.getMainIdealFilter(c_band_idx);
            switch (f.getFilterType()) {
                case zldsp::filter::kHighPass:
                case zldsp::filter::kLowShelf: {
                    bound.removeFromLeft(current_x_);
                    g.fillRect(bound);
                    break;
                }
                case zldsp::filter::kLowPass:
                case zldsp::filter::kHighShelf: {
                    bound = bound.removeFromLeft(current_x_);
                    g.fillRect(bound);
                    break;
                }
                case zldsp::filter::kTiltShelf: {
                    break;
                }
                case zldsp::filter::kPeak:
                case zldsp::filter::kBandShelf:
                case zldsp::filter::kBandPass:
                case zldsp::filter::kNotch: {
                    const auto bound_width = bound.getWidth();
                    const auto left_width = current_x_ - current_bw_ * bound_width;
                    const auto right_width = bound_width - current_x_ - current_bw_ * bound_width;
                    const auto left_area = bound.removeFromLeft(left_width);
                    const auto right_area = bound.removeFromRight(right_width);
                    g.fillRect(left_area);
                    g.fillRect(right_area);
                }
            }
        }
    }

    void SoloPanel::handleAsyncUpdate() {
        solo_q_ = soloF.getQ();
        const auto bw = std::asinh(0.5f / solo_q_);
        current_bw_ = static_cast<float>(bw) / std::log(2200.f);
    }

    void SoloPanel::parameterChanged(const juce::String &parameter_id, const float new_value) {
        juce::ignoreUnused(parameter_id);
        const auto previous_band_idx = band_idx_.load();
        const auto c_band_idx = static_cast<size_t>(new_value);
        if (previous_band_idx != c_band_idx) {
            solo_updaters_[previous_band_idx]->update(0.f);
            side_solo_updaters_[previous_band_idx]->update(0.f);
        }
        band_idx_.store(c_band_idx);
    }

    void SoloPanel::turnOffSolo() const {
        for (size_t i = 0; i < zlp::kBandNUM; ++i) {
            solo_updaters_[i]->updateSync(0.f);
            side_solo_updaters_[i]->updateSync(0.f);
        }
    }
} // zlpanel
