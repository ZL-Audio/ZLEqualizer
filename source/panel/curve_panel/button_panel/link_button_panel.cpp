// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "link_button_panel.hpp"

namespace zlPanel {
    LinkButtonPanel::LinkButtonPanel(const size_t idx,
                                     juce::AudioProcessorValueTreeState &parameters,
                                     juce::AudioProcessorValueTreeState &parametersNA,
                                     zlInterface::UIBase &base,
                                     zlInterface::Dragger &sideDragger)
        : parametersRef(parameters), parametersNARef(parametersNA),
          uiBase(base),
          sideDraggerRef(sideDragger),
          dynLinkC("L", base),
          linkDrawable(juce::Drawable::createFromImageData(BinaryData::linksfill_svg, BinaryData::linksfill_svgSize)),
          bandIdx(idx) {
        dynLinkC.getLAF().enableShadow(false);
        dynLinkC.setDrawable(linkDrawable.get());
        attach({&dynLinkC.getButton()}, {zlDSP::appendSuffix(zlDSP::singleDynLink::ID, bandIdx)},
               parameters, buttonAttachments);
        addAndMakeVisible(dynLinkC);
        sideDraggerRef.addMouseListener(this, true);

        for (auto &ID: IDs) {
            const auto suffixID = zlDSP::appendSuffix(ID, idx);
            parametersRef.addParameterListener(suffixID, this);
            parameterChanged(suffixID, parametersRef.getRawParameterValue(suffixID)->load());
        }
        for (auto &ID: NAIDs) {
            parametersNARef.addParameterListener(ID, this);
            parameterChanged(ID, parametersNARef.getRawParameterValue(ID)->load());
        }
        setInterceptsMouseClicks(false, true);
    }

    LinkButtonPanel::~LinkButtonPanel() {
        const auto idx = bandIdx.load();
        for (auto &ID: IDs) {
            parametersRef.removeParameterListener(zlDSP::appendSuffix(ID, idx), this);
        }
        for (auto &ID: NAIDs) {
            parametersNARef.removeParameterListener(ID, this);
        }
    }

    void LinkButtonPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID.startsWith(zlDSP::sideFreq::ID)) {
            sideFreq.store(newValue);
        } else if (parameterID.startsWith(zlDSP::dynamicON::ID)) {
            isDynamicON.store(newValue > .5f);
        } else if (parameterID.startsWith(zlState::selectedBandIdx::ID)) {
            isSelected.store(static_cast<size_t>(newValue) == bandIdx.load());
        }
    }

    void LinkButtonPanel::updateBound() {
        if (isSelected.load() && isDynamicON.load()) {
            const auto dynPos = static_cast<float>(sideDraggerRef.getButton().getBounds().getCentreX());
            auto buttonBound = juce::Rectangle<float>{buttonSize, buttonSize};
            buttonBound = buttonBound.withCentre({dynPos, buttonBottom});
            dynLinkC.setBounds(buttonBound.toNearestInt());
            setVisible(true);
        } else {
            setVisible(false);
        }
    }

    void LinkButtonPanel::mouseDoubleClick(const juce::MouseEvent &event) {
        if (event.mods.isCommandDown() && event.mods.isRightButtonDown()) {
            const auto currentBand = bandIdx.load();
            auto *para = parametersRef.getParameter(
                zlDSP::appendSuffix(zlDSP::sideSolo::ID, currentBand));
            para->beginChangeGesture();
            if (para->getValue() < 0.5f) {
                para->setValueNotifyingHost(1.f);
            } else {
                para->setValueNotifyingHost(0.f);
            }
            para->endChangeGesture();
        }
    }

    void LinkButtonPanel::resized() {
        buttonSize = 2.5f * uiBase.getFontSize();
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 8 * uiBase.getFontSize());
        buttonBottom = bound.getBottom();
    }
} // zlPanel
