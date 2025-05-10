// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "link_button_panel.hpp"

namespace zlpanel {
    LinkButtonPanel::LinkButtonPanel(const size_t idx,
                                     juce::AudioProcessorValueTreeState &parameters,
                                     juce::AudioProcessorValueTreeState &parameters_NA,
                                     zlgui::UIBase &base,
                                     zlgui::Dragger &sideDragger)
        : parameters_ref_(parameters), parameters_NA_ref_(parameters_NA),
          ui_base_(base),
          sideDraggerRef(sideDragger),
          dynLinkC("L", base),
          linkDrawable(juce::Drawable::createFromImageData(BinaryData::linksfill_svg, BinaryData::linksfill_svgSize)),
          bandIdx(idx) {
        dynLinkC.getLAF().enableShadow(false);
        dynLinkC.setDrawable(linkDrawable.get());
        attach({&dynLinkC.getButton()}, {zlp::appendSuffix(zlp::singleDynLink::ID, bandIdx)},
               parameters, buttonAttachments);
        addAndMakeVisible(dynLinkC);
        sideDraggerRef.addMouseListener(this, true);

        for (auto &ID: IDs) {
            const auto suffixID = zlp::appendSuffix(ID, idx);
            parameters_ref_.addParameterListener(suffixID, this);
            parameterChanged(suffixID, parameters_ref_.getRawParameterValue(suffixID)->load());
        }
        for (auto &ID: NAIDs) {
            parameters_NA_ref_.addParameterListener(ID, this);
            parameterChanged(ID, parameters_NA_ref_.getRawParameterValue(ID)->load());
        }
        setInterceptsMouseClicks(false, true);
    }

    LinkButtonPanel::~LinkButtonPanel() {
        const auto idx = bandIdx.load();
        for (auto &ID: IDs) {
            parameters_ref_.removeParameterListener(zlp::appendSuffix(ID, idx), this);
        }
        for (auto &ID: NAIDs) {
            parameters_NA_ref_.removeParameterListener(ID, this);
        }
    }

    void LinkButtonPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID.startsWith(zlp::dynamicON::ID)) {
            isDynamicON.store(newValue > .5f);
        } else if (parameterID.startsWith(zlstate::selectedBandIdx::ID)) {
            isSelected.store(static_cast<size_t>(newValue) == bandIdx.load());
        }
    }

    void LinkButtonPanel::updateBound() {
        if (isSelected.load() && isDynamicON.load()) {
            const auto dynPos = static_cast<float>(sideDraggerRef.getButton().getBoundsInParent().getCentreX());
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
            auto *para = parameters_ref_.getParameter(
                zlp::appendSuffix(zlp::sideSolo::ID, currentBand));
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
        buttonSize = 2.5f * ui_base_.getFontSize();
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 8 * ui_base_.getFontSize());
        buttonBottom = bound.getBottom();
    }
} // zlpanel
