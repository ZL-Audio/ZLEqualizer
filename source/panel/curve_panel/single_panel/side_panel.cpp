// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "side_panel.hpp"

namespace zlpanel {
    SidePanel::SidePanel(const size_t band_idx,
                         juce::AudioProcessorValueTreeState &parameters,
                         juce::AudioProcessorValueTreeState &parameters_NA,
                         zlgui::UIBase &base,
                         zlp::Controller<double> &controller,
                         zlgui::Dragger &side_dragger)
        : band_idx_(band_idx),
          parameters_ref_(parameters), parameters_NA_ref_(parameters_NA),
          ui_base_(base),
          side_f_(controller.getFilter(band_idx).getSideFilter()),
          side_dragger_ref_(side_dragger) {
        setInterceptsMouseClicks(false, false);
        const std::string suffix = zlp::appendSuffix("", band_idx_);
        parameterChanged(zlp::dynamicON::ID + suffix,
                         parameters_ref_.getRawParameterValue(zlp::dynamicON::ID + suffix)->load());
        parameterChanged(zlp::sideQ::ID + suffix,
                         parameters_ref_.getRawParameterValue(zlp::sideQ::ID + suffix)->load());
        parameterChanged(zlstate::selectedBandIdx::ID,
                         parameters_NA_ref_.getRawParameterValue(zlstate::selectedBandIdx::ID)->load());
        parameterChanged(zlstate::active::ID + suffix,
                         parameters_NA_ref_.getRawParameterValue(zlstate::active::ID + suffix)->load());

        for (auto &id: kChangeIDs) {
            parameters_ref_.addParameterListener(id + suffix, this);
        }
        parameters_NA_ref_.addParameterListener(zlstate::selectedBandIdx::ID, this);
        parameters_NA_ref_.addParameterListener(zlstate::active::ID + suffix, this);
        lookAndFeelChanged();
    }

    SidePanel::~SidePanel() {
        const std::string suffix = zlp::appendSuffix("", band_idx_);
        for (auto &id: kChangeIDs) {
            parameters_ref_.removeParameterListener(id + suffix, this);
        }
        parameters_NA_ref_.removeParameterListener(zlstate::selectedBandIdx::ID, this);
        parameters_NA_ref_.removeParameterListener(zlstate::active::ID + suffix, this);
    }

    void SidePanel::paint(juce::Graphics &g) {
        if (!isVisible()) {
            return;
        }
        const auto bound = getLocalBounds().toFloat();

        const auto x = side_dragger_ref_.getButton().getBoundsInParent().toFloat().getCentreX();
        const auto thickness = ui_base_.getFontSize() * 0.15f;
        g.setColour(colour_);
        g.drawLine(x - current_bw_, bound.getY(), x + current_bw_, bound.getY(), thickness);
    }

    void SidePanel::parameterChanged(const juce::String &parameter_id, float new_value) {
        if (parameter_id == zlstate::selectedBandIdx::ID) {
            selected_.store(static_cast<size_t>(new_value) == band_idx_);
        } else {
            if (parameter_id.startsWith(zlstate::active::ID)) {
                active_.store(new_value > .5f);
            } else if (parameter_id.startsWith(zlp::dynamicON::ID)) {
                dyn_on_.store(new_value > .5f);
            } else if (parameter_id.startsWith(zlp::sideQ::ID)) {
                side_q_.store(new_value);
                to_update_.store(true);
            }
        }
    }

    void SidePanel::updateDragger() {
        if (!selected_.load() || !active_.load() || !dyn_on_.load()) {
            setVisible(false);
            return;
        } else {
            setVisible(true);
        }
        if (to_update_.exchange(false)) {
            const auto bw = std::asinh(0.5f / side_q_.load());
            current_bw_ = static_cast<float>(bw) / std::log(2200.f) * getLocalBounds().toFloat().getWidth();
        }
    }

    void SidePanel::lookAndFeelChanged() {
        colour_ = ui_base_.getColorMap1(band_idx_);
    }
} // zlpanel
