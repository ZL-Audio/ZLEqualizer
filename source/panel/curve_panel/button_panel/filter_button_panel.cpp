// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "filter_button_panel.hpp"

namespace zlPanel {
    FilterButtonPanel::FilterButtonPanel(size_t bandIdx, juce::AudioProcessorValueTreeState &parameters,
                                         juce::AudioProcessorValueTreeState &parametersNA, zlInterface::UIBase &base)
        : parametersRef(parameters), parametersNARef(parametersNA),
          uiBase(base), dragger(base),
          buttonPopUp(bandIdx, parameters, parametersNA, base),
          band{bandIdx} {
        dragger.getLAF().setColour(uiBase.getColorMap1(bandIdx));
        for (const auto &idx: IDs) {
            const auto idxD = zlDSP::appendSuffix(idx, band.load());
            parametersRef.addParameterListener(idxD, this);
            parameterChanged(idxD, parametersRef.getRawParameterValue(idxD)->load());
        }
        for (const auto &idx: NAIDs) {
            const auto idxS = zlState::appendSuffix(idx, band.load());
            parametersNARef.addParameterListener(idxS, this);
            parameterChanged(idxS, parametersNARef.getRawParameterValue(idxS)->load());
        }
        dragger.setScale(scale);
        addAndMakeVisible(dragger);

        buttonPopUp.setAlwaysOnTop(true);
        dragger.getButton().onStateChange = [this]() {
            if (dragger.getButton().getToggleState()) {
                auto *para = parametersNARef.getParameter(zlState::selectedBandIdx::ID);
                para->beginChangeGesture();
                para->setValueNotifyingHost(zlState::selectedBandIdx::convertTo01(static_cast<int>(band.load())));
                para->endChangeGesture();

                addAndMakeVisible(buttonPopUp);
                buttonPopUp.componentMovedOrResized(dragger.getButton(), true, true);
            } else {
                buttonPopUp.setVisible(false);
                buttonPopUp.repaint();

                removeChildComponent(&buttonPopUp);
            }
        };
        dragger.setBufferedToImage(true);
        setInterceptsMouseClicks(false, true);

        dragger.getButton().addComponentListener(&buttonPopUp);
    }

    FilterButtonPanel::~FilterButtonPanel() {
        for (const auto &idx: IDs) {
            parametersRef.removeParameterListener(zlDSP::appendSuffix(idx, band.load()), this);
        }
        for (const auto &idx: NAIDs) {
            parametersNARef.removeParameterListener(zlState::appendSuffix(idx, band.load()), this);
        }
    }

    void FilterButtonPanel::resized() {
        updateBounds();
    }

    void FilterButtonPanel::setMaximumDB(const float db) {
        maximumDB.store(db);
        updateAttachment();
    }

    void FilterButtonPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        const auto parameter = parameterID.dropLastCharacters(2);
        if (parameter == zlDSP::fType::ID) {
            fType.store(static_cast<zlIIR::FilterType>(newValue));
            buttonPopUp.setFType(fType.load());
            updateAttachment();
            updateBounds();
            triggerAsyncUpdate();
        } else if (parameter == zlState::active::ID) {
            const auto f = static_cast<bool>(newValue);
            dragger.setActive(f);
            dragger.setInterceptsMouseClicks(false, f);
            if (!f) {
                removeChildComponent(&buttonPopUp);
            }
            triggerAsyncUpdate();
        }
    }

    void FilterButtonPanel::handleAsyncUpdate() {
        dragger.repaint();
        buttonPopUp.componentMovedOrResized(dragger.getButton(), true, true);
        buttonPopUp.repaint();
    }

    void FilterButtonPanel::updateAttachment() {
        const auto freqRange = juce::NormalisableRange<float>(10.f, 20000.f,
                                                              [](float rangeStart, float rangeEnd, float valueToRemap) {
                                                                  return std::exp(valueToRemap * std::log(
                                                                                 rangeEnd / rangeStart)) * rangeStart;
                                                              },
                                                              [](float rangeStart, float rangeEnd, float valueToRemap) {
                                                                  return std::log(valueToRemap / rangeStart) / std::log(
                                                                             rangeEnd / rangeStart);
                                                              });
        const auto maxDB = maximumDB.load();
        const auto gainRange = juce::NormalisableRange<float>(-maxDB, maxDB, .01f);
        switch (fType.load()) {
            case zlIIR::FilterType::peak:
            case zlIIR::FilterType::bandShelf:
            case zlIIR::FilterType::lowShelf:
            case zlIIR::FilterType::highShelf:
            case zlIIR::FilterType::tiltShelf: {
                auto *para1 = parametersRef.getParameter(zlDSP::appendSuffix(zlDSP::freq::ID, band.load()));
                auto *para2 = parametersRef.getParameter(zlDSP::appendSuffix(zlDSP::gain::ID, band.load()));
                attachment = std::make_unique<zlInterface::DraggerParameterAttach>(
                    *para1, freqRange,
                    *para2, gainRange,
                    dragger);
                attachment->enableX(true);
                attachment->enableY(true);
                attachment->sendInitialUpdate();
                break;
            }
            case zlIIR::FilterType::notch:
            case zlIIR::FilterType::lowPass:
            case zlIIR::FilterType::highPass:
            case zlIIR::FilterType::bandPass: {
                auto *para1 = parametersRef.getParameter(zlDSP::appendSuffix(zlDSP::freq::ID, band.load()));
                auto *para2 = parametersRef.getParameter(zlDSP::appendSuffix(zlDSP::gain::ID, band.load()));
                attachment = std::make_unique<zlInterface::DraggerParameterAttach>(
                    *para1, freqRange,
                    *para2, gainRange,
                    dragger);
                attachment->enableX(true);
                attachment->enableY(false);
                attachment->setY(0.5f);
                attachment->sendInitialUpdate();
                break;
            }
        }
    }

    void FilterButtonPanel::updateBounds() {
        auto bound = getLocalBounds().toFloat();
        bound.removeFromRight((1 - 0.98761596f) * bound.getWidth() - uiBase.getFontSize() * scale * .5f);
        dragger.setPadding(0.f, uiBase.getFontSize() * scale * .5f, uiBase.getFontSize() * scale * .5f,
                           uiBase.getFontSize() * scale * .5f);
        switch (fType.load()) {
            case zlIIR::FilterType::peak:
            case zlIIR::FilterType::bandShelf: {
                dragger.setBounds(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(),
                        bound.getHeight() - (2 - scale) * uiBase.getFontSize()).toNearestInt());
                break;
            }
            case zlIIR::FilterType::lowShelf:
            case zlIIR::FilterType::highShelf:
            case zlIIR::FilterType::tiltShelf: {
                dragger.setBounds(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(),
                        bound.getHeight() * .5f - (1 - scale) * uiBase.getFontSize()).toNearestInt());
                break;
            }
            case zlIIR::FilterType::notch:
            case zlIIR::FilterType::lowPass:
            case zlIIR::FilterType::highPass:
            case zlIIR::FilterType::bandPass: {
                dragger.setBounds(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(),
                        scale * uiBase.getFontSize()).toNearestInt());
                break;
            }
        }
    }

    void FilterButtonPanel::mouseDown(const juce::MouseEvent &event) {
        dragger.mouseDown(event);
    }

    void FilterButtonPanel::mouseUp(const juce::MouseEvent &event) {
        dragger.mouseUp(event);
    }

    void FilterButtonPanel::mouseDrag(const juce::MouseEvent &event) {
        dragger.mouseDrag(event);
    }
} // zlPanel
