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
        button.getButton().onClick = [this]() { resetBand(); };
        button.getLookAndFeel().setCurve(false, true, false, false);
        addAndMakeVisible(button);
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            const std::string suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            ResetComponent::parameterChanged(zlDSP::bypass::ID + suffix,
                                             parametersRef.getRawParameterValue(zlDSP::bypass::ID + suffix)->load());
            parametersRef.addParameterListener(zlDSP::bypass::ID + suffix, this);
        }
    }

    ResetComponent::~ResetComponent() {
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            const std::string suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            parametersRef.removeParameterListener(zlDSP::bypass::ID + suffix, this);
        }
    }

    void ResetComponent::resized() {
        button.getLookAndFeel().setPadding(uiBase.getFontSize() * 0.375f);
        button.setBounds(getLocalBounds());
    }

    void ResetComponent::parameterChanged(const juce::String &parameterID, float newValue) {
        // const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        if (!static_cast<bool>(newValue)) {
            const auto activeID = zlState::active::ID + parameterID.getLastCharacters(2);
            parametersNARef.getParameter(activeID)->beginChangeGesture();
            parametersNARef.getParameter(activeID)->setValueNotifyingHost(static_cast<float>(true));
            parametersNARef.getParameter(activeID)->endChangeGesture();
        }
    }

    void ResetComponent::attachGroup(const size_t idx) {
        bandIdx.store(idx);
    }

    void ResetComponent::resetBand() {
        const auto i = bandIdx.load();
        const auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
        for (size_t j = 0; j < resetDefaultVs.size(); ++j) {
            const auto resetID = resetIDs[j] + suffix;
            parametersRef.getParameter(resetID)->beginChangeGesture();
            parametersRef.getParameter(resetID)->setValueNotifyingHost(resetDefaultVs[j]);
            parametersRef.getParameter(resetID)->endChangeGesture();
        }
        const auto activeID = zlState::active::ID + suffix;
        parametersNARef.getParameter(activeID)->beginChangeGesture();
        parametersNARef.getParameter(activeID)->setValueNotifyingHost(static_cast<float>(false));
        parametersNARef.getParameter(activeID)->endChangeGesture();
    }

} // zlPanel
