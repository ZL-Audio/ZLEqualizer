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
                                   juce::AudioProcessorValueTreeState &parametersNA,
                                   zlgui::UIBase &base)
        : parametersRef(parameters),
          parametersNARef(parametersNA),
          uiBase(base),
          drawable(juce::Drawable::createFromImageData(BinaryData::xmark_svg, BinaryData::xmark_svgSize)),
          button(base, drawable.get(), nullptr, zlgui::multilingual::labels::bandOff) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        button.getButton().onClick = [this]() {
            const auto currentBand = bandIdx.load();
            const auto isCurrentBandSelected = uiBase.getIsBandSelected(currentBand);
            for(size_t idx = 0; idx < zlstate::bandNUM; ++idx) {
                if (idx == currentBand || (isCurrentBandSelected && uiBase.getIsBandSelected(idx))) {
                    const auto activeID = zlstate::appendSuffix(zlstate::active::ID, idx);
                    parametersNARef.getParameter(activeID)->beginChangeGesture();
                    parametersNARef.getParameter(activeID)->setValueNotifyingHost(static_cast<float>(false));
                    parametersNARef.getParameter(activeID)->endChangeGesture();
                }
            }
        };
        button.setPadding(.0f, .0f, .0f, .0f);
        addAndMakeVisible(button);
    }

    ResetComponent::~ResetComponent() = default;

    void ResetComponent::resized() {
        button.setBounds(getLocalBounds());
    }

    void ResetComponent::attachGroup(const size_t idx) {
        bandIdx.store(idx);
    }
} // zlpanel
