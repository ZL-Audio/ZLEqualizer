// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "reset_component.hpp"

#include "../../../state/state_definitions.hpp"

namespace zlPanel {
    ResetComponent::ResetComponent(juce::AudioProcessorValueTreeState &parameters,
                                   juce::AudioProcessorValueTreeState &parametersNA,
                                   zlInterface::UIBase &base)
        : parametersRef(parameters),
          parametersNARef(parametersNA),
          uiBase(base),
          drawable(juce::Drawable::createFromImageData(BinaryData::xmark_svg, BinaryData::xmark_svgSize)),
          button(drawable.get(), base) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        button.getButton().onClick = [this]() { resetBand(); };
        button.getLookAndFeel().setCurve(false, true, false, false);
        addAndMakeVisible(button);
    }

    ResetComponent::~ResetComponent() = default;

    void ResetComponent::resized() {
        button.getLookAndFeel().setPadding(uiBase.getFontSize() * 0.375f);
        button.setBounds(getLocalBounds());
    }

    void ResetComponent::attachGroup(const size_t idx) {
        bandIdx.store(idx);
    }

    void ResetComponent::resetBand() {
        const auto i = bandIdx.load();
        const auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
        const auto activeID = zlState::active::ID + suffix;
        parametersNARef.getParameter(activeID)->beginChangeGesture();
        parametersNARef.getParameter(activeID)->setValueNotifyingHost(static_cast<float>(false));
        parametersNARef.getParameter(activeID)->endChangeGesture();
    }

} // zlPanel
