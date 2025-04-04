// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "button_panel.hpp"

namespace zlPanel {
    ButtonPanel::ButtonPanel(PluginProcessor &processor,
                             zlInterface::UIBase &base)
        : processorRef(processor),
          parametersRef(processor.parameters), parametersNARef(processor.parametersNA),
          uiBase(base), controllerRef(processor.getController()),
          wheelSlider{
              zlInterface::SnappingSlider{base},
              zlInterface::SnappingSlider{base},
              zlInterface::SnappingSlider{base}
          } {
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            const auto suffix = zlDSP::appendSuffix("", i);
            freqUpdaters[i] = std::make_unique<zlChore::ParaUpdater>(
                parametersRef, zlDSP::freq::ID + suffix);
            gainUpdaters[i] = std::make_unique<zlChore::ParaUpdater>(
                parametersRef, zlDSP::gain::ID + suffix);
            QUpdaters[i] = std::make_unique<zlChore::ParaUpdater>(
                parametersRef, zlDSP::Q::ID + suffix);
            targetGainUpdaters[i] = std::make_unique<zlChore::ParaUpdater>(
                parametersRef, zlDSP::targetGain::ID + suffix);
            targetQUpdaters[i] = std::make_unique<zlChore::ParaUpdater>(
                parametersRef, zlDSP::targetQ::ID + suffix);
        }
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            panels[i] = std::make_unique<FilterButtonPanel>(i, processorRef, base);
            linkButtons[i] = std::make_unique<LinkButtonPanel>(
                i, parametersRef, parametersNARef, base, panels[i]->getSideDragger());
            // when main dragger is clicked, de-select target & side dragger
            panels[i]->getDragger().getButton().onStateChange = [this]() {
                const auto idx = selectBandIdx.load();
                if (panels[idx]->getDragger().getButton().getToggleState()) {
                    panels[idx]->getTargetDragger().getButton().setToggleState(false, juce::sendNotification);
                    panels[idx]->getSideDragger().getButton().setToggleState(false, juce::sendNotification);
                }
            };
            // when main dragger is clicked, de-select target & side dragger
            panels[i]->getTargetDragger().getButton().onStateChange = [this]() {
                const auto idx = selectBandIdx.load();
                if (panels[idx]->getTargetDragger().getButton().getToggleState()) {
                    panels[idx]->getDragger().getButton().setToggleState(false, juce::sendNotification);
                    panels[idx]->getSideDragger().getButton().setToggleState(false, juce::sendNotification);
                }
            };
            // when side dragger is clicked, de-select main & side dragger
            panels[i]->getSideDragger().getButton().onStateChange = [this]() {
                const auto idx = selectBandIdx.load();
                if (panels[idx]->getSideDragger().getButton().getToggleState()) {
                    panels[idx]->getDragger().getButton().setToggleState(false, juce::sendNotification);
                    panels[idx]->getTargetDragger().getButton().setToggleState(false, juce::sendNotification);
                }
            };
            // deselect the side dragger when link button is clicked
            linkButtons[i]->getButton().getButton().onClick = [this]() {
                const auto idx = selectBandIdx.load();
                if (linkButtons[idx]->getButton().getButton().getToggleState()) {
                    panels[idx]->getSideDragger().getButton().setToggleState(false, juce::sendNotification);
                }
            };
            panels[i]->addMouseListener(this, true);
        }
        for (const auto &idx: NAIDs) {
            parametersNARef.addParameterListener(idx, this);
            parameterChanged(idx, parametersNARef.getRawParameterValue(idx)->load());
        }
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            addChildComponent(panels[i].get());
            addChildComponent(linkButtons[i].get());
        }
        addAndMakeVisible(lassoComponent);
        itemsSet.addChangeListener(this);
    }

    ButtonPanel::~ButtonPanel() {
        for (const auto &idx: NAIDs) {
            parametersNARef.removeParameterListener(idx, this);
        }
        for (size_t tempIdx = 0; tempIdx < panels.size(); ++tempIdx) {
            for (const auto &idx: IDs) {
                const auto actualIdx = zlDSP::appendSuffix(idx, tempIdx);
                parametersRef.removeParameterListener(actualIdx, this);
            }
        }
        itemsSet.removeChangeListener(this);
        wheelAttachment[0].reset();
        wheelAttachment[1].reset();
        wheelAttachment[2].reset();

        stopTimer();
    }

    void ButtonPanel::paint(juce::Graphics &g) {
        if (uiBase.getColourByIdx(zlInterface::tagColour).getFloatAlpha() < 0.01f) {
            return;
        }
        const auto idx = selectBandIdx.load();
        const auto &p = panels[idx];
        if (!p->isVisible()) {
            return;
        }
        const auto bound = p->getDragger().getButtonArea();
        g.setFont(uiBase.getFontSize() * zlInterface::FontLarge);
        if (p->getDragger().getButton().getToggleState()) {
            const auto &f{controllerRef.getBaseFilter(idx)};
            const auto buttonC = p->getDragger().getButtonPos();
            const auto freqP = (buttonC.getX() - bound.getX()) / bound.getWidth();
            const auto gainP = (buttonC.getY() - bound.getY()) / bound.getHeight();
            drawFilterParas(g, f.getFilterType(), freqP, gainP, getLocalBounds().toFloat());
        } else if (p->getTargetDragger().getButton().getToggleState()) {
            const auto &f{controllerRef.getTargetFilter(idx)};
            const auto buttonC = p->getDragger().getButtonPos();
            const auto freqP = (buttonC.getX() - bound.getX()) / bound.getWidth();
            const auto gainP = (buttonC.getY() - bound.getY()) / bound.getHeight();
            drawFilterParas(g, f.getFilterType(), freqP, gainP, getLocalBounds().toFloat());
        } else if (p->getSideDragger().getButton().getToggleState()) {
            const auto &f{controllerRef.getFilter(idx).getSideFilter()};
            const auto buttonC = p->getSideDragger().getButtonPos();
            const auto freqP = (buttonC.getX() - bound.getX()) / bound.getWidth();
            const auto gainP = (buttonC.getY() - bound.getY()) / bound.getHeight();
            drawFilterParas(g, f.getFilterType(), freqP, gainP, getLocalBounds().toFloat());
        }
    }

    void ButtonPanel::drawFilterParas(juce::Graphics &g, const zlFilter::FilterType fType,
                                      const float freqP, const float gainP, const juce::Rectangle<float> &bound) {
        switch (fType) {
            case zlFilter::FilterType::peak:
            case zlFilter::FilterType::bandShelf: {
                drawGain(g, gainP, bound, freqP < .5f);
                break;
            }
            case zlFilter::FilterType::lowShelf: {
                drawGain(g, gainP, bound, true);
                break;
            }
            case zlFilter::FilterType::highShelf: {
                drawGain(g, gainP, bound, false);
                break;
            }
            case zlFilter::FilterType::tiltShelf: {
                drawGain(g, gainP * .5f, bound, false);
                break;
            }
            case zlFilter::FilterType::notch:
            case zlFilter::FilterType::lowPass:
            case zlFilter::FilterType::highPass:
            case zlFilter::FilterType::bandPass: {
                break;
            }
        }
        drawFreq(g, freqP, bound, false);
    }

    void ButtonPanel::drawFreq(juce::Graphics &g, const float freqP, const juce::Rectangle<float> &bound,
                               const bool isTop) {
        juce::ignoreUnused(isTop);
        const auto freq = std::exp(freqP * std::log(2000.f)) * 10.f;
        const auto freqString = freq < 100 ? juce::String(freq, 2, false) : juce::String(freq, 1, false);
        const auto p = std::clamp(freqP, 0.025f, 0.975f) * 0.9873247325443818f;
        auto textBound = juce::Rectangle<float>(uiBase.getFontSize() * 5, uiBase.getFontSize() * 1.5f);
        textBound = textBound.withCentre({bound.getWidth() * p, bound.getBottom() - 0.75f * uiBase.getFontSize()});
        const auto colour = uiBase.getColourByIdx(zlInterface::tagColour);
        g.setColour(uiBase.getBackgroundColor().withAlpha(colour.getFloatAlpha()));
        g.fillRect(textBound);
        g.setColour(colour);
        g.drawText(freqString, textBound, juce::Justification::centredBottom, false);
    }

    void ButtonPanel::drawGain(juce::Graphics &g, const float gainP, const juce::Rectangle<float> &bound,
                               const bool isLeft) {
        const auto gain = (-2.f * gainP + 1.f) * maximumDB.load();
        const auto tempBound = bound.withSizeKeepingCentre(bound.getWidth(),
                                                           bound.getHeight() - 2 * uiBase.getFontSize());
        const auto gString = std::abs(gain) < 10 ? juce::String(gain, 2, false) : juce::String(gain, 1, false);
        auto textBound = juce::Rectangle<float>(uiBase.getFontSize() * 2.7f, uiBase.getFontSize() * 1.5f);
        if (isLeft) {
            textBound = textBound.withCentre({
                uiBase.getFontSize() * 1.35f,
                tempBound.getY() + gainP * tempBound.getHeight()
            });
        } else {
            textBound = textBound.withCentre({
                tempBound.getRight() - uiBase.getFontSize() * 1.35f,
                tempBound.getY() + gainP * tempBound.getHeight()
            });
        }
        const auto colour = uiBase.getColourByIdx(zlInterface::tagColour);
        g.setColour(uiBase.getBackgroundColor().withAlpha(colour.getFloatAlpha()));
        g.fillRect(textBound);
        g.setColour(colour);
        g.drawText(gString, textBound, juce::Justification::centred, false);
    }

    void ButtonPanel::resized() {
        for (const auto &p: panels) {
            p->setBounds(getLocalBounds());
        }
        for (const auto &p: linkButtons) {
            p->setBounds(getLocalBounds());
        }
    }

    void ButtonPanel::mouseEnter(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        startTimer(500);
    }

    void ButtonPanel::mouseExit(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        stopTimer();
    }

    void ButtonPanel::timerCallback() {
        uiBase.closeAllBox();
        stopTimer();
    }

    void ButtonPanel::mouseDown(const juce::MouseEvent &event) {
        uiBase.closeAllBox();
        if (event.originalComponent != this) {
            isLeftClick.store(!event.mods.isRightButtonDown());
            return;
        }
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            panels[i]->setSelected(false);
        }

        itemsSet.deselectAll();
        previousLassoNum = 0;
        lassoComponent.setColour(juce::LassoComponent<size_t>::lassoFillColourId,
                                 uiBase.getTextColor().withMultipliedAlpha(.25f));
        lassoComponent.setColour(juce::LassoComponent<size_t>::lassoOutlineColourId,
                                 uiBase.getTextColor().withMultipliedAlpha(.375f));
        lassoComponent.setVisible(true);
        lassoComponent.beginLasso(event, this);
    }

    void ButtonPanel::mouseUp(const juce::MouseEvent &event) {
        if (event.originalComponent != this) {
            return;
        }
        lassoComponent.endLasso();
    }

    void ButtonPanel::mouseDrag(const juce::MouseEvent &event) {
        if (event.originalComponent != this) {
            return;
        }
        lassoComponent.dragLasso(event);
    }

    void ButtonPanel::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) {
        juce::MouseEvent e{
            event.source, event.position,
            event.mods.withoutMouseButtons(),
            event.pressure, event.orientation, event.rotation,
            event.tiltX, event.tiltY,
            event.eventComponent, event.originalComponent,
            event.eventTime, event.mouseDownPosition, event.mouseDownTime,
            event.getNumberOfClicks(), false
        };
        if (event.originalComponent == this) {
            wheelSlider[0].mouseWheelMove(e, wheel);
            wheelSlider[1].mouseWheelMove(e, wheel);
        } else if (!panels[selectBandIdx.load()]->isParentOf(event.originalComponent)) {
            wheelSlider[0].mouseWheelMove(e, wheel);
            wheelSlider[1].mouseWheelMove(e, wheel);
        } else {
            const auto &p = panels[selectBandIdx.load()];
            if (p.get()->getDragger().isParentOf(event.originalComponent)) {
                wheelSlider[0].mouseWheelMove(e, wheel);
            } else if (p.get()->getTargetDragger().isParentOf(event.originalComponent)) {
                wheelSlider[1].mouseWheelMove(e, wheel);
            } else if (p.get()->getSideDragger().isParentOf(event.originalComponent)) {
                if (!p.get()->getSideDragger().getButton().getToggleState()) {
                    p.get()->getSideDragger().getButton().setToggleState(true, juce::sendNotificationSync);
                }
                wheelSlider[2].mouseWheelMove(e, wheel);
            }
        }
    }

    void ButtonPanel::mouseDoubleClick(const juce::MouseEvent &event) {
        if (event.originalComponent != this) {
            return;
        }
        const auto idx = findAvailableBand();
        if (idx >= zlState::bandNUM) {
            return;
        }
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());
        const auto point = event.getPosition().toFloat();
        const auto x = point.getX(), y = point.getY();
        const auto freq = xtoFreq(x, bound);
        const auto db = yToDB(y, maximumDB.load(), bound);

        std::vector<std::string> initIDs;
        std::vector<float> initValues;

        if (freq < 20.f) {
            initIDs.emplace_back(zlDSP::fType::ID);
            initValues.emplace_back(zlDSP::fType::convertTo01(zlFilter::FilterType::highPass));
            initIDs.emplace_back(zlDSP::slope::ID);
            initValues.emplace_back(zlDSP::slope::convertTo01(uiBase.getDefaultPassFilterSlope()));
        } else if (freq < 50.f) {
            initIDs.emplace_back(zlDSP::fType::ID);
            initValues.emplace_back(zlDSP::fType::convertTo01(zlFilter::FilterType::lowShelf));
            initIDs.emplace_back(zlDSP::gain::ID);
            initValues.emplace_back(zlDSP::gain::convertTo01(
                juce::jlimit(-maximumDB.load(), maximumDB.load(), 2 * db)
            ));
        } else if (freq < 5000.f) {
            initIDs.emplace_back(zlDSP::fType::ID);
            initValues.emplace_back(zlDSP::fType::convertTo01(zlFilter::FilterType::peak));
            initIDs.emplace_back(zlDSP::gain::ID);
            initValues.emplace_back(zlDSP::gain::convertTo01(
                juce::jlimit(-maximumDB.load(), maximumDB.load(), db)
            ));
        } else if (freq < 15000.f) {
            initIDs.emplace_back(zlDSP::fType::ID);
            initValues.emplace_back(zlDSP::fType::convertTo01(zlFilter::FilterType::highShelf));
            initIDs.emplace_back(zlDSP::gain::ID);
            initValues.emplace_back(zlDSP::gain::convertTo01(
                juce::jlimit(-maximumDB.load(), maximumDB.load(), 2 * db)
            ));
        } else {
            initIDs.emplace_back(zlDSP::fType::ID);
            initValues.emplace_back(zlDSP::fType::convertTo01(zlFilter::FilterType::lowPass));
            initIDs.emplace_back(zlDSP::slope::ID);
            initValues.emplace_back(zlDSP::slope::convertTo01(uiBase.getDefaultPassFilterSlope()));
        }
        initIDs.emplace_back(zlDSP::freq::ID);
        initValues.emplace_back(zlDSP::freq::convertTo01(freq));
        initIDs.emplace_back(zlDSP::Q::ID);
        initValues.emplace_back(zlDSP::Q::convertTo01(zlDSP::Q::defaultV));
        initIDs.emplace_back(zlDSP::bypass::ID);
        initValues.emplace_back(zlDSP::bypass::convertTo01(false));
        // turn on dynamic is command is down
        if (event.mods.isCommandDown()) {
            initIDs.emplace_back(zlDSP::dynamicON::ID);
            initValues.emplace_back(zlDSP::dynamicON::convertTo01(true));
            initIDs.emplace_back(zlDSP::singleDynLink::ID);
            initValues.emplace_back(zlDSP::singleDynLink::convertTo01(uiBase.getDynLink()));
        }

        for (size_t i = 0; i < initIDs.size(); ++i) {
            const auto paraID = zlDSP::appendSuffix(initIDs[i], idx);
            auto *para = parametersRef.getParameter(paraID);
            para->beginChangeGesture();
            para->setValueNotifyingHost(initValues[i]);
            para->endChangeGesture();
        }

        if (event.mods.isCommandDown()) {
            processorRef.getFiltersAttach().turnOnDynamic(idx);
        }

        if (idx != selectBandIdx.load()) {
            auto *para = parametersNARef.getParameter(zlState::selectedBandIdx::ID);
            para->beginChangeGesture();
            para->setValueNotifyingHost(zlState::selectedBandIdx::convertTo01(static_cast<int>(idx)));
            para->endChangeGesture();
        } else {
            toAttachGroup.store(true);
        }
    }

    void ButtonPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlState::selectedBandIdx::ID) {
            const auto idx = static_cast<size_t>(newValue);
            selectBandIdx.store(idx);
            toAttachGroup.store(true);
        } else if (parameterID == zlState::maximumDB::ID) {
            const auto idx = static_cast<size_t>(newValue);
            for (const auto &p: panels) {
                p->setMaximumDB(zlState::maximumDB::dBs[idx]);
            }
            maximumDB.store(zlState::maximumDB::dBs[idx]);
        } else {
            // the parameter is freq/gain/Q/targetGain/targetQ
            if (!isDuringLasso.load()) return;
            const auto currentBand = selectBandIdx.load();
            if (!uiBase.getIsBandSelected(currentBand)) return;
            const auto value = static_cast<double>(newValue);
            if (parameterID.startsWith(zlDSP::freq::ID)) {
                const auto ratio = static_cast<float>(value / previousFreqs[currentBand].load());
                for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                    if (idx != currentBand && uiBase.getIsBandSelected(idx)) {
                        const auto shiftFreq = previousFreqs[idx].load() * ratio;
                        const auto legalFreq = zlDSP::freq::range.snapToLegalValue(shiftFreq);
                        freqUpdaters[idx]->update(zlDSP::freq::range.convertTo0to1(legalFreq));
                    }
                }
            } else if (parameterID.startsWith(zlDSP::gain::ID)) {
                if (isLeftClick.load()) {
                    if (std::abs(previousGains[currentBand].load()) <= 0.1f) return;
                    const auto scale = newValue / previousGains[currentBand].load();
                    for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                        if (idx != currentBand && uiBase.getIsBandSelected(idx)) {
                            const auto shiftGain = scale * previousGains[idx].load();
                            const auto legalGain = juce::jlimit(-maximumDB.load(), maximumDB.load(), shiftGain);
                            gainUpdaters[idx]->update(zlDSP::gain::convertTo01(legalGain));
                        }
                    }
                } else {
                    const auto shift = newValue - previousGains[currentBand].load();
                    for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                        if (idx != currentBand && uiBase.getIsBandSelected(idx)) {
                            const auto shiftGain = shift + previousGains[idx].load();
                            const auto legalGain = juce::jlimit(-maximumDB.load(), maximumDB.load(), shiftGain);
                            gainUpdaters[idx]->update(zlDSP::gain::convertTo01(legalGain));
                        }
                    }
                }
            } else if (parameterID.startsWith(zlDSP::Q::ID)) {
                const auto ratio = static_cast<float>(value / previousQs[currentBand].load());
                for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                    if (idx != currentBand && uiBase.getIsBandSelected(idx)) {
                        const auto shiftQ = ratio * previousQs[idx].load();
                        const auto legalQ = zlDSP::Q::range.snapToLegalValue(shiftQ);
                        QUpdaters[idx]->update(zlDSP::Q::range.convertTo0to1(legalQ));
                    }
                }
            } else if (parameterID.startsWith(zlDSP::targetGain::ID)) {
                if (isLeftClick.load()) {
                    if (std::abs(previousTargetGains[currentBand].load()) <= 0.1f) return;
                    const auto scale = newValue / previousTargetGains[currentBand].load();
                    for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                        if (idx != currentBand && uiBase.getIsBandSelected(idx)) {
                            const auto shiftGain = scale * previousTargetGains[idx].load();
                            const auto legalGain = juce::jlimit(-maximumDB.load(), maximumDB.load(), shiftGain);
                            targetGainUpdaters[idx]->update(zlDSP::targetGain::convertTo01(legalGain));
                        }
                    }
                } else {
                    const auto shift = newValue - previousTargetGains[currentBand].load();
                    for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                        if (idx != currentBand && uiBase.getIsBandSelected(idx)) {
                            const auto shiftGain = shift + previousTargetGains[idx].load();
                            const auto legalGain = juce::jlimit(-maximumDB.load(), maximumDB.load(), shiftGain);
                            targetGainUpdaters[idx]->update(zlDSP::targetGain::convertTo01(legalGain));
                        }
                    }
                }
            } else if (parameterID.startsWith(zlDSP::targetQ::ID)) {
                const auto ratio = static_cast<float>(value / previousTargetQs[currentBand].load());
                for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                    if (idx != currentBand && uiBase.getIsBandSelected(idx)) {
                        const auto shiftQ = ratio * previousTargetQs[idx].load();
                        const auto legalQ = zlDSP::Q::range.snapToLegalValue(shiftQ);
                        targetQUpdaters[idx]->update(zlDSP::targetQ::range.convertTo0to1(legalQ));
                    }
                }
            }
        }
    }

    void ButtonPanel::attachGroup(const size_t idx) {
        loadPreviousParameters();
        for (size_t oldIdx = 0; oldIdx < zlState::bandNUM; ++oldIdx) {
            for (const auto &parameter: IDs) {
                parametersRef.removeParameterListener(zlDSP::appendSuffix(parameter, oldIdx), this);
            }
        }
        for (const auto &parameter: IDs) {
            parametersRef.addParameterListener(zlDSP::appendSuffix(parameter, idx), this);
        }
    }

    void ButtonPanel::updateAttach() {
        if (toAttachGroup.exchange(false)) {
            const auto idx = selectBandIdx.load();
            attachGroup(idx);
            for (size_t i = 0; i < zlState::bandNUM; ++i) {
                panels[i]->setSelected(i == idx);
            }
            panels[idx]->toFront(false);
            wheelAttachment[0].reset();
            wheelAttachment[0] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                parametersRef, zlDSP::appendSuffix(zlDSP::Q::ID, selectBandIdx.load()), wheelSlider[0]);
            wheelAttachment[1].reset();
            wheelAttachment[1] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                parametersRef, zlDSP::appendSuffix(zlDSP::targetQ::ID, selectBandIdx.load()), wheelSlider[1]);
            wheelAttachment[2].reset();
            wheelAttachment[2] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                parametersRef, zlDSP::appendSuffix(zlDSP::sideQ::ID, selectBandIdx.load()), wheelSlider[2]);
        }
    }

    size_t ButtonPanel::findAvailableBand() const {
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            const auto idx = zlState::appendSuffix(zlState::active::ID, i);
            const auto isActive = parametersNARef.getRawParameterValue(idx)->load() > .5f;
            if (!isActive) { return i; }
        }
        return zlState::bandNUM;
    }

    void ButtonPanel::findLassoItemsInArea(juce::Array<size_t> &itemsFound, const juce::Rectangle<int> &area) {
        juce::ignoreUnused(itemsFound, area);
        const auto floatArea = area.toFloat();
        for (size_t idx = 0; idx < panels.size(); ++idx) {
            if (panels[idx]->isVisible()) {
                const auto transform = panels[idx]->getDragger().getButton().getTransform();
                const auto dPosition = panels[idx]->getDragger().getPosition().toFloat();
                const auto bCenter = dPosition.transformedBy(transform);
                if (floatArea.contains(bCenter)) {
                    itemsFound.add(idx);
                }
            }
        }
    }

    juce::SelectedItemSet<size_t> &ButtonPanel::getLassoSelection() {
        return itemsSet;
    }

    void ButtonPanel::changeListenerCallback(juce::ChangeBroadcaster *source) {
        juce::ignoreUnused(source);
        int currentSelectedNum = 0;
        size_t currentFirstSelectIdx = 0;
        const auto currentBand = selectBandIdx.load();
        bool isCurrentBandSelected = false;
        for (size_t idx = 0; idx < panels.size(); ++idx) {
            const auto f1 = itemsSet.isSelected(idx);
            if (f1) {
                if (currentSelectedNum == 0) {
                    currentFirstSelectIdx = idx;
                }
                if (idx == currentBand) {
                    isCurrentBandSelected = true;
                }
                currentSelectedNum += 1;
            }
            uiBase.setIsBandSelected(idx, f1);
            const auto f2 = panels[idx]->getDragger().getLAF().getIsSelected();
            if (f1 != f2) {
                panels[idx]->getDragger().getLAF().setIsSelected(f1);
                panels[idx]->getDragger().getButton().repaint();
            }
        }
        if (currentSelectedNum > 0) {
            if (previousLassoNum == 0 || !isCurrentBandSelected) {
                panels[currentFirstSelectIdx]->setSelected(true);
            }
            previousLassoNum = currentSelectedNum;
            loadPreviousParameters();
            isDuringLasso.store(true);
        } else {
            isDuringLasso.store(false);
        }
    }

    void ButtonPanel::loadPreviousParameters() {
        for (size_t idx = 0; idx < panels.size(); ++idx) {
            previousFreqs[idx].store(zlDSP::freq::range.convertFrom0to1(
                freqUpdaters[idx]->getPara()->getValue()));
            previousGains[idx].store(zlDSP::gain::range.convertFrom0to1(
                gainUpdaters[idx]->getPara()->getValue()));
            previousQs[idx].store(zlDSP::Q::range.convertFrom0to1(
                QUpdaters[idx]->getPara()->getValue()));
            previousTargetGains[idx].store(zlDSP::targetGain::range.convertFrom0to1(
                targetGainUpdaters[idx]->getPara()->getValue()));
            previousTargetQs[idx].store(zlDSP::targetQ::range.convertFrom0to1(
                targetQUpdaters[idx]->getPara()->getValue()));
        }
    }
} // zlPanel
