// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "filter_button_panel.hpp"

namespace zlpanel {
    FilterButtonPanel::FilterButtonPanel(const size_t bandIdx, PluginProcessor &processor, zlgui::UIBase &base)
        : processor_ref_(processor),
          parameters_ref_(processor.parameters), parameters_NA_ref_(processor.parameters_NA),
          uiBase(base),
          dragger(base), targetDragger(base), sideDragger(base),
          buttonPopUp(bandIdx, parameters_ref_, parameters_NA_ref_, base),
          band{bandIdx},
          currentSelectedBandIdx(*parameters_NA_ref_.getRawParameterValue(zlstate::selectedBandIdx::ID)) {
        dragger.addMouseListener(this, true);
        dragger.getButton().setBufferedToImage(true);
        dragger.setBroughtToFrontOnMouseClick(true);
        targetDragger.getLAF().setDraggerShape(zlgui::DraggerLookAndFeel::DraggerShape::upDownArrow);
        targetDragger.setBroughtToFrontOnMouseClick(true);
        sideDragger.getLAF().setDraggerShape(zlgui::DraggerLookAndFeel::DraggerShape::rectangle);
        lookAndFeelChanged();
        for (const auto &idx: IDs) {
            const auto idxD = zlp::appendSuffix(idx, band);
            parameters_ref_.addParameterListener(idxD, this);
            parameterChanged(idxD, parameters_ref_.getRawParameterValue(idxD)->load());
        }
        for (const auto &idx: NAIDs) {
            const auto idxS = zlstate::appendSuffix(idx, band);
            parameters_NA_ref_.addParameterListener(idxS, this);
            parameterChanged(idxS, parameters_NA_ref_.getRawParameterValue(idxS)->load());
        }
        parameters_NA_ref_.addParameterListener(zlstate::selectedBandIdx::ID, this);
        parameterChanged(zlstate::selectedBandIdx::ID,
                         parameters_NA_ref_.getRawParameterValue(zlstate::selectedBandIdx::ID)->load());

        for (auto &d: {&sideDragger, &targetDragger, &dragger}) {
            d->setScale(scale);
            addAndMakeVisible(d);
        }
        addChildComponent(buttonPopUp);
        // set current band if dragger is clicked
        dragger.getButton().onClick = [this]() {
            if (dragger.getButton().getToggleState()) {
                if (static_cast<size_t>(currentSelectedBandIdx.load()) != band) {
                    auto *para = parameters_NA_ref_.getParameter(zlstate::selectedBandIdx::ID);
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(zlstate::selectedBandIdx::convertTo01(static_cast<int>(band)));
                    para->endChangeGesture();
                }
                buttonPopUp.toFront(false);
                buttonPopUp.setVisible(true);
            } else {
                buttonPopUp.setVisible(false);
            }
        };
        // disable link if side dragger is clicked
        sideDragger.getButton().onClick = [this]() {
            if (sideDragger.getButton().getToggleState()) {
                const auto para = parameters_ref_.
                        getParameter(zlp::appendSuffix(zlp::singleDynLink::ID, band));
                para->beginChangeGesture();
                para->setValueNotifyingHost(0.f);
                para->endChangeGesture();
            }
        };

        setInterceptsMouseClicks(false, true);
        dragger.setInterceptsMouseClicks(false, true);
        targetDragger.setInterceptsMouseClicks(false, true);
        sideDragger.setInterceptsMouseClicks(false, true);
    }

    FilterButtonPanel::~FilterButtonPanel() {
        for (const auto &idx: IDs) {
            parameters_ref_.removeParameterListener(zlp::appendSuffix(idx, band), this);
        }
        for (const auto &idx: NAIDs) {
            parameters_NA_ref_.removeParameterListener(zlstate::appendSuffix(idx, band), this);
        }
        parameters_NA_ref_.removeParameterListener(zlstate::selectedBandIdx::ID, this);
    }

    void FilterButtonPanel::resized() {
        updateBounds();
    }

    void FilterButtonPanel::setMaximumDB(const float db) {
        maximumDB.store(db);
        toUpdateAttachment.store(true);
        toUpdateTargetAttachment.store(true);
        toUpdateDraggers.store(true);
    }

    void FilterButtonPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlstate::selectedBandIdx::ID) {
            isSelectedTarget.store(static_cast<size_t>(newValue) == band);
            toUpdateTargetAttachment.store(true);
            toUpdateDraggers.store(true);
            return;
        }
        if (parameterID.startsWith(zlp::fType::ID)) {
            fType.store(static_cast<zldsp::filter::FilterType>(newValue));

            toUpdateAttachment.store(true);
            toUpdateTargetAttachment.store(true);
            toUpdateBounds.store(true);
            toUpdateDraggers.store(true);
        } else if (parameterID.startsWith(zlstate::active::ID)) {
            const auto f = newValue > .5f;
            isActiveTarget.store(f);
            toUpdateTargetAttachment.store(true);
            toUpdateDraggers.store(true);
        } else if (parameterID.startsWith(zlp::dynamicON::ID)) {
            isDynamicHasTarget.store(newValue > .5f);
            toUpdateTargetAttachment.store(true);
            toUpdateDraggers.store(true);
        } else if (parameterID.startsWith(zlp::lrType::ID)) {
            lrType.store(static_cast<zlp::lrType::lrTypes>(newValue));
            toUpdateDraggerLabel.store(true);
            toUpdateDraggers.store(true);
        }
    }

    void FilterButtonPanel::handleAsyncUpdate() {
        const auto f = isActiveTarget.load();
        setVisible(f);
        dragger.setVisible(f);
        dragger.getButton().setToggleState(isSelectedTarget.load(), juce::sendNotificationSync);
        if (toUpdateAttachment.exchange(false)) {
            updateAttachment();
        }
        if (toUpdateTargetAttachment.exchange(false)) {
            updateTargetAttachment();
        }
        if (toUpdateDraggerLabel.exchange(false)) {
            updateDraggerLabel();
        }
        if (toUpdateBounds.exchange(false)) {
            updateBounds();
        }
        dragger.getButton().repaint();
    }

    void FilterButtonPanel::updateAttachment() {
        const auto maxDB = maximumDB.load();
        const auto gainRange = juce::NormalisableRange<float>(-maxDB, maxDB, .01f);
        switch (fType.load()) {
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kBandShelf:
            case zldsp::filter::FilterType::kLowShelf:
            case zldsp::filter::FilterType::kHighShelf:
            case zldsp::filter::FilterType::kTiltShelf: {
                auto *para1 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::freq::ID, band));
                auto *para2 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::gain::ID, band));
                attachment = std::make_unique<zlgui::DraggerParameterAttach>(
                    *para1, freqRange,
                    *para2, gainRange,
                    dragger);
                attachment->enableX(true);
                attachment->enableY(true);
                attachment->sendInitialUpdate();
                break;
            }
            case zldsp::filter::FilterType::kNotch:
            case zldsp::filter::FilterType::kLowPass:
            case zldsp::filter::FilterType::kHighPass:
            case zldsp::filter::FilterType::kBandPass: {
                auto *para1 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::freq::ID, band));
                auto *para2 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::gain::ID, band));
                attachment = std::make_unique<zlgui::DraggerParameterAttach>(
                    *para1, freqRange,
                    *para2, gainRange,
                    dragger);
                attachment->enableX(true);
                attachment->enableY(false); {
                    attachment->setY(0.5f);
                }
                attachment->sendInitialUpdate();
                break;
            }
        }
    }

    void FilterButtonPanel::updateBounds() {
        dragger.setBounds(getLocalBounds());
        targetDragger.setBounds(getLocalBounds());
        sideDragger.setBounds(getLocalBounds());
        auto bound = getLocalBounds().toFloat();
        bound.removeFromRight((1 - 0.98761596f) * bound.getWidth());
        switch (fType.load()) {
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kBandShelf: {
                dragger.setButtonArea(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(),
                        bound.getHeight() - 2 * uiBase.getFontSize()));
                targetDragger.setButtonArea(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(),
                        bound.getHeight() - 2 * uiBase.getFontSize()));
                break;
            }
            case zldsp::filter::FilterType::kLowShelf:
            case zldsp::filter::FilterType::kHighShelf:
            case zldsp::filter::FilterType::kTiltShelf: {
                dragger.setButtonArea(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(),
                        bound.getHeight() * .5f - 1 * uiBase.getFontSize()));
                targetDragger.setButtonArea(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(),
                        bound.getHeight() * .5f - 1 * uiBase.getFontSize()));
                break;
            }
            case zldsp::filter::FilterType::kNotch:
            case zldsp::filter::FilterType::kLowPass:
            case zldsp::filter::FilterType::kHighPass:
            case zldsp::filter::FilterType::kBandPass: {
                dragger.setButtonArea(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(),
                        scale * uiBase.getFontSize()));
                break;
            }
        }
        juce::Rectangle<float> sideBound{
            bound.getX(), bound.getBottom() - 2 * uiBase.getFontSize() - .5f * scale * uiBase.getFontSize(),
            bound.getWidth(), scale * uiBase.getFontSize()
        };
        sideBound = sideBound.withSizeKeepingCentre(sideBound.getWidth(), 1.f);
        sideDragger.setButtonArea(sideBound);
    }

    void FilterButtonPanel::updateTargetAttachment() {
        bool isFilterTypeHasTarget = false;
        switch (fType.load()) {
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kBandShelf:
            case zldsp::filter::FilterType::kLowShelf:
            case zldsp::filter::FilterType::kHighShelf:
            case zldsp::filter::FilterType::kTiltShelf: {
                isFilterTypeHasTarget = true;
                break;
            }
            case zldsp::filter::FilterType::kNotch:
            case zldsp::filter::FilterType::kLowPass:
            case zldsp::filter::FilterType::kHighPass:
            case zldsp::filter::FilterType::kBandPass: {
                isFilterTypeHasTarget = false;
                break;
            }
        }
        if (isDynamicHasTarget.load() && isFilterTypeHasTarget &&
            isSelectedTarget.load() && isActiveTarget.load()) {
            const auto maxDB = maximumDB.load();
            const auto gainRange = juce::NormalisableRange<float>(-maxDB, maxDB, .01f);
            auto *para1 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::freq::ID, band));
            auto *para3 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::targetGain::ID, band));
            targetAttach = std::make_unique<zlgui::DraggerParameterAttach>(
                *para1, freqRange,
                *para3, gainRange,
                targetDragger);
            targetAttach->enableX(true);
            targetAttach->enableY(true);
            targetAttach->sendInitialUpdate();
            targetDragger.setVisible(true);
        } else {
            targetAttach.reset();
            targetDragger.setVisible(false);
        }
        if (isDynamicHasTarget.load() && isSelectedTarget.load() && isActiveTarget.load()) {
            const auto maxDB = maximumDB.load();
            const auto gainRange = juce::NormalisableRange<float>(-maxDB, maxDB, .01f);
            auto *para2 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::sideFreq::ID, band));
            auto *para3 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::targetGain::ID, band));
            sideAttach = std::make_unique<zlgui::DraggerParameterAttach>(
                *para2, freqRange,
                *para3, gainRange,
                sideDragger);
            sideAttach->enableX(true);
            sideAttach->enableY(false);
            sideAttach->sendInitialUpdate();
            sideDragger.setVisible(true);
        } else {
            sideAttach.reset();
            sideDragger.setVisible(false);
        }
    }

    void FilterButtonPanel::updateDraggerLabel() {
        switch (lrType.load()) {
            case zlp::lrType::kStereo:
                dragger.getLAF().setLabel("");
            break;
            case zlp::lrType::kLeft:
                dragger.getLAF().setLabel("L");
            break;
            case zlp::lrType::kRight:
                dragger.getLAF().setLabel("R");
            break;
            case zlp::lrType::kMid:
                dragger.getLAF().setLabel("M");
            break;
            case zlp::lrType::kSide:
                dragger.getLAF().setLabel("S");
            break;
        }
    }

    void FilterButtonPanel::setSelected(const bool f) {
        if (dragger.getButton().getToggleState() != f) {
            dragger.getButton().setToggleState(f, juce::NotificationType::sendNotificationSync);
        }
        if (targetDragger.getButton().getToggleState()) {
            targetDragger.getButton().setToggleState(false, juce::NotificationType::dontSendNotification);
        }
        if (sideDragger.getButton().getToggleState()) {
            sideDragger.getButton().setToggleState(false, juce::NotificationType::dontSendNotification);
        }
    }

    void FilterButtonPanel::mouseDoubleClick(const juce::MouseEvent &event) {
        if (event.mods.isCommandDown()) {
            const auto currentBand = band;
            if (event.mods.isLeftButtonDown()) {
                // turn on/off current band dynamic
                const auto paraID = zlp::appendSuffix(zlp::dynamicON::ID, currentBand);
                const auto newValue = 1.f - parameters_ref_.getRawParameterValue(paraID)->load(); {
                    auto *para = parameters_ref_.getParameter(paraID);
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(newValue);
                    para->endChangeGesture();
                }
                float dynLinkValue = 0.0;
                if (newValue > 0.5f) {
                    processor_ref_.getFiltersAttach().turnOnDynamic(currentBand);
                    dynLinkValue = static_cast<float>(uiBase.getDynLink());
                } else {
                    processor_ref_.getFiltersAttach().turnOffDynamic(currentBand);
                } {
                    auto *para = parameters_ref_.getParameter(
                        zlp::appendSuffix(zlp::singleDynLink::ID, currentBand));
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(dynLinkValue);
                    para->endChangeGesture();
                }
            } else if (event.mods.isRightButtonDown()) {
                auto *para = parameters_ref_.getParameter(
                    zlp::appendSuffix(zlp::solo::ID, currentBand));
                para->beginChangeGesture();
                if (para->getValue() < 0.5f) {
                    para->setValueNotifyingHost(1.f);
                } else {
                    para->setValueNotifyingHost(0.f);
                }
                para->endChangeGesture();
            }
        }
    }

    void FilterButtonPanel::lookAndFeelChanged() {
        for (auto &d: {&sideDragger, &targetDragger, &dragger}) {
            d->getLAF().setColour(uiBase.getColorMap1(band));
        }
    }

    void FilterButtonPanel::visibilityChanged() {
        if (!isVisible()) {
            buttonPopUp.setBounds({
                std::numeric_limits<int>::min() / 2, std::numeric_limits<int>::min() / 2,
                buttonPopUp.getBounds().getWidth(), buttonPopUp.getBounds().getHeight()
            });
        }
    }
} // zlpanel
