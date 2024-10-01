// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "button_panel.hpp"

namespace zlPanel {
    ButtonPanel::ButtonPanel(PluginProcessor &processor,
                             zlInterface::UIBase &base,
                             zlDSP::Controller<double> &c)
        : processorRef(processor),
          parametersRef(processor.parameters), parametersNARef(processor.parametersNA),
          uiBase(base), controllerRef(c),
          wheelSlider{
              zlInterface::SnappingSlider{base},
              zlInterface::SnappingSlider{base},
              zlInterface::SnappingSlider{base}
          } {
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            panels[i] = std::make_unique<FilterButtonPanel>(i, processorRef, base);
            linkButtons[i] = std::make_unique<LinkButtonPanel>(i, parametersRef, parametersNARef, base);
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
            addAndMakeVisible(panels[i].get());
            addAndMakeVisible(linkButtons[i].get());
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
    }

    void ButtonPanel::paint(juce::Graphics &g) {
        if (uiBase.getColourByIdx(zlInterface::tagColour).getFloatAlpha() < 0.01f) {
            return;
        }
        const auto bound = getLocalBounds().toFloat();
        const auto idx = selectBandIdx.load();
        const auto &p = panels[idx];
        if (!p->getDragger().getLAF().getActive()) {
            return;
        }
        g.setFont(uiBase.getFontSize() * zlInterface::FontLarge);
        if (p->getDragger().getButton().getToggleState()) {
            const auto &f{controllerRef.getBaseFilter(idx)};
            drawFilterParas(g, f.getFilterType(), f.getFreq(), f.getGain(), bound);
        } else if (p->getTargetDragger().getButton().getToggleState()) {
            const auto &f{controllerRef.getTargetFilter(idx)};
            drawFilterParas(g, f.getFilterType(), f.getFreq(), f.getGain(), bound);
        } else if (p->getSideDragger().getButton().getToggleState()) {
            const auto &f{controllerRef.getFilter(idx).getSideFilter()};
            drawFilterParas(g, f.getFilterType(), f.getFreq(), f.getGain(), bound);
        }
    }

    void ButtonPanel::drawFilterParas(juce::Graphics &g, const zlFilter::FilterType fType,
                                      const double freq, const double gain, const juce::Rectangle<float> &bound) {
        switch (fType) {
            case zlFilter::FilterType::peak:
            case zlFilter::FilterType::bandShelf: {
                drawGain(g, static_cast<float>(gain), bound, static_cast<float>(freq) <= 500.f);
                break;
            }
            case zlFilter::FilterType::lowShelf: {
                drawGain(g, static_cast<float>(gain), bound, true);
                break;
            }
            case zlFilter::FilterType::highShelf: {
                drawGain(g, static_cast<float>(gain), bound, false);
                break;
            }
            case zlFilter::FilterType::tiltShelf: {
                drawGain(g, static_cast<float>(gain) * .5f, bound, false);
                break;
            }
            case zlFilter::FilterType::notch:
            case zlFilter::FilterType::lowPass:
            case zlFilter::FilterType::highPass:
            case zlFilter::FilterType::bandPass: {
                break;
            }
        }
        drawFreq(g, static_cast<float>(freq), bound, false);
    }

    void ButtonPanel::drawFreq(juce::Graphics &g, const float freq, const juce::Rectangle<float> &bound,
                               const bool isTop) {
        juce::ignoreUnused(isTop);
        const auto freqString = freq < 100 ? juce::String(freq, 2, false) : juce::String(freq, 1, false);
        auto p = std::log(freq / 10.f) / std::log(2205.f);
        p = juce::jlimit(0.025f, 0.97f, p);
        auto textBound = juce::Rectangle<float>(uiBase.getFontSize() * 5, uiBase.getFontSize() * 1.5f);
        textBound = textBound.withCentre({bound.getWidth() * p, bound.getBottom() - 0.75f * uiBase.getFontSize()});
        const auto colour = uiBase.getColourByIdx(zlInterface::tagColour);
        g.setColour(uiBase.getBackgroundColor().withAlpha(colour.getFloatAlpha()));
        g.fillRect(textBound);
        g.setColour(colour);
        g.drawText(freqString, textBound, juce::Justification::centredBottom, false);
    }

    void ButtonPanel::drawGain(juce::Graphics &g, const float gain, const juce::Rectangle<float> &bound,
                               const bool isLeft) {
        const auto tempBound = bound.withSizeKeepingCentre(bound.getWidth(),
                                                           bound.getHeight() - 2 * uiBase.getFontSize());
        const auto gString = std::abs(gain) < 10 ? juce::String(gain, 2, false) : juce::String(gain, 1, false);
        const auto p = juce::jlimit(-0.5f, 0.5f, -.5f * gain / maximumDB.load());
        auto textBound = juce::Rectangle<float>(uiBase.getFontSize() * 2.7f, uiBase.getFontSize() * 1.5f);
        if (isLeft) {
            textBound = textBound.withCentre({
                uiBase.getFontSize() * 1.35f,
                tempBound.getY() + (0.5f + p) * tempBound.getHeight()
            });
        } else {
            textBound = textBound.withCentre({
                tempBound.getRight() - uiBase.getFontSize() * 1.35f,
                tempBound.getY() + (0.5f + p) * tempBound.getHeight()
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

    void ButtonPanel::mouseDown(const juce::MouseEvent &event) {
        if (event.originalComponent != this) {
            isLeftClick.store(!event.mods.isRightButtonDown());
            for (size_t idx = 0; idx < panels.size(); ++idx) {
                previousGains[idx].store(
                    static_cast<float>(controllerRef.getBaseFilter(idx).getGain()));
            }
            return;
        }
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            panels[i]->setSelected(false);
        }

        itemsSet.deselectAll();
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
        }

        for (size_t i = 0; i < initIDs.size(); ++i) {
            const auto paraID = zlDSP::appendSuffix(initIDs[i], idx);
            auto *_para = parametersRef.getParameter(paraID);
            _para->beginChangeGesture();
            _para->setValueNotifyingHost(initValues[i]);
            _para->endChangeGesture();
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
            triggerAsyncUpdate();
        }
    }

    void ButtonPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlState::selectedBandIdx::ID) {
            const auto idx = static_cast<size_t>(newValue);
            selectBandIdx.store(idx);
            toAttachGroup.store(true);
            triggerAsyncUpdate();
        } else if (parameterID == zlState::maximumDB::ID) {
            const auto idx = static_cast<size_t>(newValue);
            for (const auto &p: panels) {
                p->setMaximumDB(zlState::maximumDB::dBs[idx]);
            }
            maximumDB.store(zlState::maximumDB::dBs[idx]);
        } else {
            // the parameter is freq/gain/Q
            const auto currentBand = selectBandIdx.load();
            const auto value = static_cast<double>(newValue);
            if (parameterID.startsWith(zlDSP::freq::ID)) {
                const auto ratio = static_cast<float>(value / currentFreq.load());
                currentFreq.store(value);
                if (!uiBase.getIsBandSelected(currentBand)) return;
                for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                    if (idx != currentBand && uiBase.getIsBandSelected(idx)) {
                        auto *para = parametersRef.getParameter(zlDSP::appendSuffix(zlDSP::freq::ID, idx));
                        const auto shiftFreq = para->convertFrom0to1(para->getValue()) * ratio;
                        const auto legalFreq = zlDSP::freq::range.snapToLegalValue(shiftFreq);
                        para->beginChangeGesture();
                        para->setValueNotifyingHost(para->convertTo0to1(legalFreq));
                        para->endChangeGesture();
                    }
                }
            } else if (parameterID.startsWith(zlDSP::gain::ID)) {
                if (!uiBase.getIsBandSelected(currentBand)) return;
                if (isLeftClick.load()) {
                    if (std::abs(previousGains[currentBand].load()) <= 0.1f) return;
                    const auto scale = newValue / previousGains[currentBand].load();
                    for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                        if (idx != currentBand && uiBase.getIsBandSelected(idx)) {
                            auto *para = parametersRef.getParameter(zlDSP::appendSuffix(zlDSP::gain::ID, idx));
                            const auto shiftGain = scale * previousGains[idx].load();
                            const auto legalGain = juce::jlimit(-maximumDB.load(), maximumDB.load(), shiftGain);
                            para->beginChangeGesture();
                            para->setValueNotifyingHost(para->convertTo0to1(legalGain));
                            para->endChangeGesture();
                        }
                    }
                } else {
                    const auto shift = newValue - previousGains[currentBand].load();
                    for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                        if (idx != currentBand && uiBase.getIsBandSelected(idx)) {
                            auto *para = parametersRef.getParameter(zlDSP::appendSuffix(zlDSP::gain::ID, idx));
                            const auto shiftGain = shift + previousGains[idx].load();
                            const auto legalGain = juce::jlimit(-maximumDB.load(), maximumDB.load(), shiftGain);
                            para->beginChangeGesture();
                            para->setValueNotifyingHost(para->convertTo0to1(legalGain));
                            para->endChangeGesture();
                        }
                    }
                }
            } else if (parameterID.startsWith(zlDSP::Q::ID)) {
                const auto ratio = static_cast<float>(value / currentQ.load());
                currentQ.store(value);
                if (!uiBase.getIsBandSelected(currentBand)) return;
                for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                    if (idx != currentBand && uiBase.getIsBandSelected(idx)) {
                        auto *para = parametersRef.getParameter(zlDSP::appendSuffix(zlDSP::Q::ID, idx));
                        const auto shiftQ = para->convertFrom0to1(para->getValue()) * ratio;
                        const auto legalQ = zlDSP::Q::range.snapToLegalValue(shiftQ);
                        para->beginChangeGesture();
                        para->setValueNotifyingHost(para->convertTo0to1(legalQ));
                        para->endChangeGesture();
                    }
                }
            }
        }
    }

    void ButtonPanel::attachGroup(const size_t idx) {
        for (size_t oldIdx = 0; oldIdx < zlState::bandNUM; ++oldIdx) {
            for (const auto &parameter: IDs) {
                parametersRef.removeParameterListener(zlDSP::appendSuffix(parameter, oldIdx), this);
            }
        }
        for (const auto &parameter: IDs) {
            parametersRef.addParameterListener(zlDSP::appendSuffix(parameter, idx), this);
        }
        currentFreq.store(
            static_cast<double>(parametersRef.getRawParameterValue(zlDSP::appendSuffix(zlDSP::freq::ID, idx))->load()));
        currentQ.store(
            static_cast<double>(parametersRef.getRawParameterValue(zlDSP::appendSuffix(zlDSP::Q::ID, idx))->load()));
    }

    void ButtonPanel::handleAsyncUpdate() {
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
            panels[idx]->getTargetDragger().getButton().setToggleState(false, juce::sendNotification);
            panels[idx]->getSideDragger().getButton().setToggleState(false, juce::sendNotification);
        }
        repaint();
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
        for (size_t idx = 0; idx < panels.size(); ++idx) {
            if (parametersNARef.getRawParameterValue(
                zlState::appendSuffix(zlState::active::ID, idx))->load() > .5f) {
                auto bCenter = panels[idx]->getDragger().getButton().getBounds().toFloat().getCentre();
                const auto dPosition = panels[idx]->getDragger().getPosition().toFloat();
                bCenter = bCenter.translated(dPosition.getX(), dPosition.getY());
                if (area.contains(bCenter.roundToInt())) {
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
        for (size_t idx = 0; idx < panels.size(); ++idx) {
            const auto f1 = itemsSet.isSelected(idx);
            uiBase.setIsBandSelected(idx, f1);
            const auto f2 = panels[idx]->getDragger().getLAF().getIsSelected();
            if (f1 != f2) {
                panels[idx]->getDragger().getLAF().setIsSelected(f1);
                panels[idx]->repaint();
            }
        }
    }
} // zlPanel
