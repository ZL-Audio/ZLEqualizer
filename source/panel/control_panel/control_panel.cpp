// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "control_panel.hpp"

#include "../../state/state_definitions.hpp"

namespace zlpanel {
    ControlPanel::ControlPanel(PluginProcessor &p,
                               zlgui::UIBase &base)
        : parameters_ref_(p.parameters_), parameters_NA_ref_(p.parameters_NA_), ui_base_(base),
          left_control_panel_(p, base),
          right_control_panel_(p, base),
          match_control_panel_(p, base) {
        addAndMakeVisible(left_control_panel_);
        addChildComponent(right_control_panel_);
        addChildComponent(match_control_panel_);
        for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
            const auto idx = zlp::appendSuffix(zlp::dynamicON::ID, i);
            dynamic_on_[i].store(parameters_ref_.getRawParameterValue(idx)->load() > .5f);
            parameters_ref_.addParameterListener(idx, this);
        }
        parameterChanged(zlstate::selectedBandIdx::ID,
                         parameters_NA_ref_.getRawParameterValue(zlstate::selectedBandIdx::ID)->load());
        parameters_NA_ref_.addParameterListener(zlstate::selectedBandIdx::ID, this);

        setOpaque(true);
    }

    ControlPanel::~ControlPanel() {
        parameters_NA_ref_.removeParameterListener(zlstate::selectedBandIdx::ID, this);
        for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
            const auto idx = zlp::appendSuffix(zlp::dynamicON::ID, i);
            parameters_ref_.removeParameterListener(idx, this);
        }
    }

    void ControlPanel::paint(juce::Graphics &g) {
        g.fillAll(ui_base_.getBackgroundColor());
    }

    void ControlPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        const auto actual_bound = ui_base_.getRoundedShadowRectangleArea(bound, ui_base_.getFontSize(), {});

        const auto left_width = actual_bound.getWidth() * (33.f / 63.f) + (bound.getWidth() - actual_bound.getWidth()) *
                               .5f;
        auto right_bound = getLocalBounds();
        const auto left_bound = right_bound.removeFromLeft(juce::roundToInt(left_width));
        left_control_panel_.setBounds(left_bound);
        right_control_panel_.setBounds(right_bound);
        match_control_panel_.setBounds(getLocalBounds());
    }

    void ControlPanel::parameterChanged(const juce::String &parameter_id, const float new_value) {
        if (parameter_id == zlstate::selectedBandIdx::ID) {
            band_idx_.store(static_cast<size_t>(new_value));
            triggerAsyncUpdate();
        } else {
            const auto idx = static_cast<size_t>(parameter_id.getTrailingIntValue());
            dynamic_on_[idx].store(new_value > .5f);
            if (idx == band_idx_.load()) {
                triggerAsyncUpdate();
            }
        }
    }

    void ControlPanel::handleAsyncUpdate() {
        const auto idx = band_idx_.load();
        left_control_panel_.attachGroup(idx);
        right_control_panel_.attachGroup(idx);
        const auto f = dynamic_on_[idx].load();
        left_control_panel_.getDynamicAutoButton().setVisible(f);
        right_control_panel_.setVisible(f);
    }
} // zlpanel
