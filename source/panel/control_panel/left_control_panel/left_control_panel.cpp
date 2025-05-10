// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "left_control_panel.hpp"

namespace zlpanel {
    LeftControlPanel::LeftControlPanel(PluginProcessor &p,
                                       zlgui::UIBase &base)
        : processor_ref_(p), uiBase(base),
          parameters_ref_(p.parameters),
          parameters_NA_ref_(p.parameters_NA),
          background(uiBase),
          bypassC("B", base, zlgui::multilingual::labels::bandBypass),
          soloC("S", base, zlgui::multilingual::labels::bandSolo),
          dynONC("D", base, zlgui::multilingual::labels::bandDynamic),
          dynLC("L", base, zlgui::multilingual::labels::bandDynamicAuto),
          fTypeC("", zlp::fType::choices, base, zlgui::multilingual::labels::bandType),
          slopeC("", zlp::slope::choices, base, zlgui::multilingual::labels::bandSlope),
          stereoC("", zlp::lrType::choices, base, zlgui::multilingual::labels::bandStereoMode),
          lrBox(zlstate::selectedBandIdx::choices, base, zlgui::multilingual::labels::bandSelector),
          freqC("FREQ", base, zlgui::multilingual::labels::bandFreq),
          gainC("GAIN", base, zlgui::multilingual::labels::bandGain),
          qC("Q", base, zlgui::multilingual::labels::bandQ),
          resetComponent(p.parameters, p.parameters_NA, base),
          bypassDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg, BinaryData::fadpowerswitch_svgSize)),
          soloDrawable(juce::Drawable::createFromImageData(BinaryData::fadsolo_svg, BinaryData::fadsolo_svgSize)),
          dynONDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadmodsine_svg, BinaryData::fadmodsine_svgSize)),
          dynLeDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpreseta_svg, BinaryData::fadpreseta_svgSize)) {
        juce::ignoreUnused(parameters_NA_ref_);
        addAndMakeVisible(background);
        bypassC.setDrawable(bypassDrawable.get());
        bypassC.getLAF().setReverse(true);
        bypassC.getButton().onClick = [this]() {
            const auto isByPassed = static_cast<float>(bypassC.getButton().getToggleState());
            const auto currentBand = bandIdx.load();
            const auto isCurrentBandSelected = uiBase.getIsBandSelected(currentBand);
            for (size_t idx = 0; idx < zlstate::bandNUM; ++idx) {
                if (idx == currentBand || (isCurrentBandSelected && uiBase.getIsBandSelected(idx))) {
                    const auto paraBypass = parameters_ref_.getParameter(zlstate::appendSuffix(zlp::bypass::ID, idx));
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
                processor_ref_.getFiltersAttach().turnOnDynamic(currentBand);
                dynLinkValue = static_cast<float>(uiBase.getDynLink()); {
                    auto *para = parameters_ref_.getParameter(
                        zlp::appendSuffix(zlp::bypass::ID, currentBand));
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(0.f);
                    para->endChangeGesture();
                }
            } else {
                processor_ref_.getFiltersAttach().turnOffDynamic(currentBand);
            } {
                auto *para = parameters_ref_.getParameter(
                    zlp::appendSuffix(zlp::singleDynLink::ID, currentBand));
                para->beginChangeGesture();
                para->setValueNotifyingHost(dynLinkValue);
                para->endChangeGesture();
            }
        };

        dynLC.setDrawable(dynLeDrawable.get());
        dynLC.getButton().onClick = [this]() {
            const auto currentBand = bandIdx.load();
            if (dynLC.getButton().getToggleState()) {
                processor_ref_.getFiltersAttach().turnOnDynamicAuto(currentBand);
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
        for (size_t i = 0; i < zlp::kBandNUM; ++i) {
            const std::string suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            parameters_ref_.removeParameterListener(zlp::fType::ID + suffix, this);
            parameters_ref_.removeParameterListener(zlp::dynamicON::ID + suffix, this);
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
        parameters_ref_.removeParameterListener(zlp::fType::ID + oldSuffix, this);
        parameters_ref_.removeParameterListener(zlp::dynamicON::ID + oldSuffix, this);

        bandIdx.store(idx);
        resetComponent.attachGroup(idx);
        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        parameters_ref_.addParameterListener(zlp::fType::ID + suffix, this);
        parameters_ref_.addParameterListener(zlp::dynamicON::ID + suffix, this);

        buttonAttachments.clear(true);
        boxAttachments.clear(true);
        sliderAttachments.clear(true);

        attach({&bypassC.getButton(), &soloC.getButton(), &dynONC.getButton(), &dynLC.getButton()},
               {
                   zlp::bypass::ID + suffix, zlp::solo::ID + suffix,
                   zlp::dynamicON::ID + suffix, zlp::dynamicLearn::ID + suffix
               },
               parameters_ref_, buttonAttachments);
        attach({&fTypeC.getBox(), &slopeC.getBox(), &stereoC.getBox()},
               {zlp::fType::ID + suffix, zlp::slope::ID + suffix, zlp::lrType::ID + suffix},
               parameters_ref_, boxAttachments);
        attach({&lrBox.getBox()},
               {zlstate::selectedBandIdx::ID},
               parameters_NA_ref_, boxAttachments);
        attach({&freqC.getSlider1(), &gainC.getSlider1(), &gainC.getSlider2(), &qC.getSlider1(), &qC.getSlider2()},
               {
                   zlp::freq::ID + suffix, zlp::gain::ID + suffix, zlp::targetGain::ID + suffix,
                   zlp::Q::ID + suffix, zlp::targetQ::ID + suffix
               },
               parameters_ref_, sliderAttachments);
        freqC.updateDisplay();
        gainC.updateDisplay();
        qC.updateDisplay();
        parameterChanged(zlp::fType::ID + suffix,
                         parameters_ref_.getRawParameterValue(zlp::fType::ID + suffix)->load());
        parameterChanged(zlp::dynamicON::ID + suffix,
                         parameters_ref_.getRawParameterValue(zlp::dynamicON::ID + suffix)->load());
    }

    void LeftControlPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        if (parameterID.startsWith(zlp::fType::ID)) {
            switch (static_cast<zldsp::filter::FilterType>(newValue)) {
                case zldsp::filter::FilterType::kPeak:
                case zldsp::filter::FilterType::kLowShelf:
                case zldsp::filter::FilterType::kHighShelf:
                case zldsp::filter::FilterType::kBandShelf:
                case zldsp::filter::FilterType::kTiltShelf: {
                    gainCEditable.store(true);
                    break;
                }
                case zldsp::filter::FilterType::kLowPass:
                case zldsp::filter::FilterType::kHighPass:
                case zldsp::filter::FilterType::kBandPass:
                case zldsp::filter::FilterType::kNotch: {
                    gainCEditable.store(false);
                    break;
                }
            }
            switch (static_cast<zldsp::filter::FilterType>(newValue)) {
                case zldsp::filter::FilterType::kLowPass:
                case zldsp::filter::FilterType::kHighPass:
                case zldsp::filter::FilterType::kLowShelf:
                case zldsp::filter::FilterType::kHighShelf:
                case zldsp::filter::FilterType::kBandShelf:
                case zldsp::filter::FilterType::kTiltShelf: {
                    slopCEnable.store(true);
                    break;
                }
                case zldsp::filter::FilterType::kPeak:
                case zldsp::filter::FilterType::kBandPass:
                case zldsp::filter::FilterType::kNotch: {
                    slopCEnable.store(false);
                    break;
                }
            }
            if (idx == bandIdx.load()) {
                triggerAsyncUpdate();
            }
        } else if (parameterID.startsWith(zlp::dynamicON::ID)) {
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
