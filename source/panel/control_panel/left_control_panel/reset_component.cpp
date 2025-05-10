// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "reset_component.hpp"

#include "../../../state/state_definitions.hpp"

namespace zlpanel {
    ResetComponent::ResetComponent(juce::AudioProcessorValueTreeState &parameters,
                                   juce::AudioProcessorValueTreeState &parameters_NA,
                                   zlgui::UIBase &base)
        : parameters_ref_(parameters),
          parameters_NA_ref_(parameters_NA),
          ui_base_(base),
          drawable_(juce::Drawable::createFromImageData(BinaryData::xmark_svg, BinaryData::xmark_svgSize)),
          button_(base, drawable_.get(), nullptr, zlgui::multilingual::Labels::kBandOff) {
        juce::ignoreUnused(parameters_ref_, parameters_NA_ref_);
        button_.getButton().onClick = [this]() {
            const auto currentBand = band_idx_.load();
            const auto isCurrentBandSelected = ui_base_.getIsBandSelected(currentBand);
            for(size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
                if (idx == currentBand || (isCurrentBandSelected && ui_base_.getIsBandSelected(idx))) {
                    const auto activeID = zlstate::appendSuffix(zlstate::active::ID, idx);
                    parameters_NA_ref_.getParameter(activeID)->beginChangeGesture();
                    parameters_NA_ref_.getParameter(activeID)->setValueNotifyingHost(static_cast<float>(false));
                    parameters_NA_ref_.getParameter(activeID)->endChangeGesture();
                }
            }
        };
        button_.setPadding(.0f, .0f, .0f, .0f);
        addAndMakeVisible(button_);
    }

    ResetComponent::~ResetComponent() = default;

    void ResetComponent::resized() {
        button_.setBounds(getLocalBounds());
    }

    void ResetComponent::attachGroup(const size_t idx) {
        band_idx_.store(idx);
    }
} // zlpanel
