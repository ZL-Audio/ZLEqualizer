// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "left_control_panel.hpp"

namespace zlPanel {
    LeftControlPanel::LeftControlPanel(PluginProcessor &p,
                                       zlInterface::UIBase &base)
        : processorRef(p), uiBase(base),
          parametersRef(p.parameters),
          parametersNARef(p.parametersNA),
          background(uiBase),
          bypassC("B", base, zlInterface::multilingual::labels::bandBypass),
          soloC("S", base, zlInterface::multilingual::labels::bandSolo),
          dynONC("D", base, zlInterface::multilingual::labels::bandDynamic),
          dynLC("L", base, zlInterface::multilingual::labels::bandDynamicAuto),
          fTypeC("", zlDSP::fType::choices, base, zlInterface::multilingual::labels::bandType),
          slopeC("", zlDSP::slope::choices, base, zlInterface::multilingual::labels::bandSlope),
          stereoC("", zlDSP::lrType::choices, base, zlInterface::multilingual::labels::bandStereoMode),
          lrBox(zlState::selectedBandIdx::choices, base, zlInterface::multilingual::labels::bandSelector),
          freqC("FREQ", base, zlInterface::multilingual::labels::bandFreq),
          gainC("GAIN", base, zlInterface::multilingual::labels::bandGain),
          qC("Q", base, zlInterface::multilingual::labels::bandQ),
          resetComponent(p.parameters, p.parametersNA, base),
          bypassDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg, BinaryData::fadpowerswitch_svgSize)),
          soloDrawable(juce::Drawable::createFromImageData(BinaryData::fadsolo_svg, BinaryData::fadsolo_svgSize)),
          dynONDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadmodsine_svg, BinaryData::fadmodsine_svgSize)),
          dynLeDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpreseta_svg, BinaryData::fadpreseta_svgSize)) {
        juce::ignoreUnused(parametersNARef);
        addAndMakeVisible(background);
        bypassC.setDrawable(bypassDrawable.get());
        bypassC.getLAF().setReverse(true);
        bypassC.getButton().onClick = [this]() {
            const auto isByPassed = static_cast<float>(bypassC.getButton().getToggleState());
            const auto currentBand = bandIdx.load();
            const auto isCurrentBandSelected = uiBase.getIsBandSelected(currentBand);
            for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                if (idx == currentBand || (isCurrentBandSelected && uiBase.getIsBandSelected(idx))) {
                    const auto paraBypass = parametersRef.getParameter(zlState::appendSuffix(zlDSP::bypass::ID, idx));
                    paraBypass->beginChangeGesture();
                    paraBypass->setValueNotifyingHost(isByPassed);
                    paraBypass->endChangeGesture();
                }
            }
        };

        soloC.setDrawable(soloDrawable.get());
        dynONC.setDrawable(dynONDrawable.get());
        dynONC.getButton().onClick = [this]() {
            const auto currentBand = bandIdx.load();
            float dynLinkValue = 0.0;
            if (dynONC.getButton().getToggleState()) {
                processorRef.getFiltersAttach().turnOnDynamic(currentBand);
                dynLinkValue = static_cast<float>(uiBase.getDynLink()); {
                    auto *para = parametersRef.getParameter(
                        zlDSP::appendSuffix(zlDSP::bypass::ID, currentBand));
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(0.f);
                    para->endChangeGesture();
                }
            } else {
                processorRef.getFiltersAttach().turnOffDynamic(currentBand);
            } {
                auto *para = parametersRef.getParameter(
                    zlDSP::appendSuffix(zlDSP::singleDynLink::ID, currentBand));
                para->beginChangeGesture();
                para->setValueNotifyingHost(dynLinkValue);
                para->endChangeGesture();
            }
        };

        dynLC.setDrawable(dynLeDrawable.get());
        dynLC.getButton().onClick = [this]() {
            const auto currentBand = bandIdx.load();
            if (dynLC.getButton().getToggleState()) {
                processorRef.getFiltersAttach().turnOnDynamicAuto(currentBand);
            }
        };

        for (auto &c: {&bypassC, &soloC, &dynONC, &dynLC}) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
        }
        for (auto &c: {&fTypeC, &slopeC, &stereoC}) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
        }
        for (auto &c: {&freqC}) {
            addAndMakeVisible(c);
        }
        freqC.parseString = [](const juce::String s) {
            if (const auto v = parseFreqPitchString(s)) {
                return v.value();
            } else {
                return parseFreqValueString(s);
            }
        };
        freqC.allowedChars = juce::String("0123456789.kKABCDEFGabcdefg#");
        qC.setBufferedToImage(true);
        for (auto &c: {&gainC, &qC}) {
            addAndMakeVisible(c);
        }
        gainC.allowedChars = juce::String("-0123456789.");
        qC.allowedChars = juce::String("0123456789.");
        lrBox.setBufferedToImage(true);
        addAndMakeVisible(lrBox);
        resetComponent.setBufferedToImage(true);
        addAndMakeVisible(resetComponent);
    }

    LeftControlPanel::~LeftControlPanel() {
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            const std::string suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            parametersRef.removeParameterListener(zlDSP::fType::ID + suffix, this);
            parametersRef.removeParameterListener(zlDSP::dynamicON::ID + suffix, this);
        }
    }

    void LeftControlPanel::resized() {
        // update padding
        {
            for (auto &s: {&freqC}) {
                s->setPadding(std::round(uiBase.getFontSize() * 0.5f), 0.f);
            }
            for (auto &s: {&gainC, &qC}) {
                s->setPadding(std::round(uiBase.getFontSize() * 0.5f), 0.f);
            }
        }
        // update bounds
        auto bound = getLocalBounds();
        background.setBounds(bound); {
            const auto pad = static_cast<int>(uiBase.getFontSize() * .5f);
            bound = bound.withSizeKeepingCentre(bound.getWidth() - 2 * pad, bound.getHeight() - 2 * pad);
        }
        const auto buttonWidth = static_cast<int>(uiBase.getFontSize() * buttonWidthP);
        const auto buttonHeight = std::min(static_cast<int>(uiBase.getFontSize() * buttonHeightP),
                                           bound.getHeight() / 2);
        const auto sliderWidth = static_cast<int>(std::round(uiBase.getFontSize() * rSliderWidthP)); {
            auto mBound = bound.removeFromLeft(buttonWidth);
            bypassC.setBounds(mBound.removeFromTop(buttonHeight));
            soloC.setBounds(mBound.removeFromBottom(buttonHeight));
        } {
            auto mBound = bound.removeFromLeft(bound.getWidth() - (buttonWidth * 2 + sliderWidth * 3));
            const auto boxHeight = mBound.getHeight() / 3;
            fTypeC.setBounds(mBound.removeFromTop(boxHeight));
            slopeC.setBounds(mBound.removeFromTop(boxHeight));
            stereoC.setBounds(mBound.removeFromTop(boxHeight));
        }
        freqC.setBounds(bound.removeFromLeft(sliderWidth));
        gainC.setBounds(bound.removeFromLeft(sliderWidth));
        qC.setBounds(bound.removeFromLeft(sliderWidth));
        const auto resetWidth = juce::roundToInt(1.5f * uiBase.getFontSize()); {
            auto mBound = bound.removeFromBottom(buttonHeight);
            dynONC.setBounds(mBound.removeFromLeft(buttonWidth));
            dynLC.setBounds(mBound);
            mBound = bound;
            mBound.removeFromLeft(juce::roundToInt(0.5f * uiBase.getFontSize()));
            mBound.removeFromRight(juce::roundToInt(1.75f * uiBase.getFontSize()));
            lrBox.setBounds(mBound);
        } {
            bound = bound.removeFromRight(resetWidth);
            bound = bound.removeFromTop(resetWidth);
            resetComponent.setBounds(bound);
        }
        // update sliders' dragging distance
        updateMouseDragSensitivity();
    }

    void LeftControlPanel::lookAndFeelChanged() {
        updateMouseDragSensitivity();
    }

    void LeftControlPanel::attachGroup(const size_t idx) {
        const std::string oldSuffix = bandIdx.load() < 10
                                          ? "0" + std::to_string(bandIdx.load())
                                          : std::to_string(bandIdx.load());
        parametersRef.removeParameterListener(zlDSP::fType::ID + oldSuffix, this);
        parametersRef.removeParameterListener(zlDSP::dynamicON::ID + oldSuffix, this);

        bandIdx.store(idx);
        resetComponent.attachGroup(idx);
        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        parametersRef.addParameterListener(zlDSP::fType::ID + suffix, this);
        parametersRef.addParameterListener(zlDSP::dynamicON::ID + suffix, this);

        buttonAttachments.clear(true);
        boxAttachments.clear(true);
        sliderAttachments.clear(true);

        attach({&bypassC.getButton(), &soloC.getButton(), &dynONC.getButton(), &dynLC.getButton()},
               {
                   zlDSP::bypass::ID + suffix, zlDSP::solo::ID + suffix,
                   zlDSP::dynamicON::ID + suffix, zlDSP::dynamicLearn::ID + suffix
               },
               parametersRef, buttonAttachments);
        attach({&fTypeC.getBox(), &slopeC.getBox(), &stereoC.getBox()},
               {zlDSP::fType::ID + suffix, zlDSP::slope::ID + suffix, zlDSP::lrType::ID + suffix},
               parametersRef, boxAttachments);
        attach({&lrBox.getBox()},
               {zlState::selectedBandIdx::ID},
               parametersNARef, boxAttachments);
        attach({&freqC.getSlider1(), &gainC.getSlider1(), &gainC.getSlider2(), &qC.getSlider1(), &qC.getSlider2()},
               {
                   zlDSP::freq::ID + suffix, zlDSP::gain::ID + suffix, zlDSP::targetGain::ID + suffix,
                   zlDSP::Q::ID + suffix, zlDSP::targetQ::ID + suffix
               },
               parametersRef, sliderAttachments);
        freqC.updateDisplay();
        gainC.updateDisplay();
        qC.updateDisplay();
        parameterChanged(zlDSP::fType::ID + suffix,
                         parametersRef.getRawParameterValue(zlDSP::fType::ID + suffix)->load());
        parameterChanged(zlDSP::dynamicON::ID + suffix,
                         parametersRef.getRawParameterValue(zlDSP::dynamicON::ID + suffix)->load());
    }

    void LeftControlPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        if (parameterID.startsWith(zlDSP::fType::ID)) {
            switch (static_cast<zlFilter::FilterType>(newValue)) {
                case zlFilter::FilterType::peak:
                case zlFilter::FilterType::lowShelf:
                case zlFilter::FilterType::highShelf:
                case zlFilter::FilterType::bandShelf:
                case zlFilter::FilterType::tiltShelf: {
                    gainCEditable.store(true);
                    break;
                }
                case zlFilter::FilterType::lowPass:
                case zlFilter::FilterType::highPass:
                case zlFilter::FilterType::bandPass:
                case zlFilter::FilterType::notch: {
                    gainCEditable.store(false);
                    break;
                }
            }
            switch (static_cast<zlFilter::FilterType>(newValue)) {
                case zlFilter::FilterType::lowPass:
                case zlFilter::FilterType::highPass:
                case zlFilter::FilterType::lowShelf:
                case zlFilter::FilterType::highShelf:
                case zlFilter::FilterType::bandShelf:
                case zlFilter::FilterType::tiltShelf: {
                    slopCEnable.store(true);
                    break;
                }
                case zlFilter::FilterType::peak:
                case zlFilter::FilterType::bandPass:
                case zlFilter::FilterType::notch: {
                    slopCEnable.store(false);
                    break;
                }
            }
            if (idx == bandIdx.load()) {
                triggerAsyncUpdate();
            }
        } else if (parameterID.startsWith(zlDSP::dynamicON::ID)) {
            const auto f = newValue > .5f; {
                gainS2Editable.store(gainCEditable.load() && f);
                qS2Editable.store(f);
            }
            if (idx == bandIdx.load()) {
                triggerAsyncUpdate();
            }
        }
    }

    void LeftControlPanel::handleAsyncUpdate() {
        gainC.setEditable(gainCEditable.load());
        slopeC.getBox().setItemEnabled(1, slopCEnable.load());
        gainC.setShowSlider2(gainS2Editable.load());
        qC.setShowSlider2(qS2Editable.load());
        repaint();
    }

    void LeftControlPanel::updateMouseDragSensitivity() {
        const auto style = uiBase.getRotaryStyle();
        const auto sensitivity = juce::roundToInt(uiBase.getRotaryDragSensitivity() * uiBase.getFontSize());
        for (auto &c: {&freqC}) {
            c->getSlider1().setSliderStyle(style);
            c->getSlider2().setSliderStyle(style);
            c->setMouseDragSensitivity(sensitivity);
        }
        for (auto &c: {&gainC, &qC}) {
            c->getSlider1().setSliderStyle(style);
            c->getSlider2().setSliderStyle(style);
            c->setMouseDragSensitivity(sensitivity);
        }
    }
}
