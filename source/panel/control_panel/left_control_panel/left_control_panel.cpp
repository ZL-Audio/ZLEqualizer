// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "left_control_panel.hpp"

namespace zlPanel {
    LeftControlPanel::LeftControlPanel(juce::AudioProcessorValueTreeState &parameters,
                                       juce::AudioProcessorValueTreeState &parametersNA,
                                       zlInterface::UIBase &base)
        : uiBase(base),
          parametersRef(parameters),
          parametersNARef(parametersNA),
          bypassC("B", base),
          soloC("S", base), dynONC("D", base), dynLC("L", base),
          fTypeC("", zlDSP::fType::choices, base),
          slopeC("", zlDSP::slope::choices, base),
          stereoC("", zlDSP::lrType::choices, base),
          lrBox(zlState::selectedBandIdx::choices, base),
          freqC("FREQ", base),
          gainC("GAIN", base),
          qC("Q", base),
          resetComponent(parameters, parametersNA, base),
          bypassDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg, BinaryData::fadpowerswitch_svgSize)),
          soloDrawable(juce::Drawable::createFromImageData(BinaryData::fadsolo_svg, BinaryData::fadsolo_svgSize)),
          dynONDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadmodsine_svg, BinaryData::fadmodsine_svgSize)),
          dynLeDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpreseta_svg, BinaryData::fadpreseta_svgSize)) {
        juce::ignoreUnused(parametersNA, parametersNARef);
        bypassC.setDrawable(bypassDrawable.get());
        bypassC.getLAF().setReverse(true);
        soloC.setDrawable(soloDrawable.get());
        dynONC.setDrawable(dynONDrawable.get());
        dynLC.setDrawable(dynLeDrawable.get());
        for (auto &c: {&bypassC, &soloC, &dynONC, &dynLC}) {
            addAndMakeVisible(c);
        }
        for (auto &c: {&fTypeC, &slopeC, &stereoC}) {
            addAndMakeVisible(c);
        }
        for (auto &c: {&freqC, &gainC, &qC}) {
            addAndMakeVisible(c);
        }
        addAndMakeVisible(lrBox);
        addAndMakeVisible(resetComponent);
    }

    LeftControlPanel::~LeftControlPanel() {
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            const std::string suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            parametersRef.removeParameterListener(zlDSP::fType::ID + suffix, this);
            parametersRef.removeParameterListener(zlDSP::dynamicON::ID + suffix, this);
        }
    }

    void LeftControlPanel::paint(juce::Graphics &g) {
        const auto bound = getLocalBounds().toFloat();
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
        const auto style = uiBase.getRotaryStyle();
        const auto sensitivity = juce::roundToInt(uiBase.getRotaryDragSensitivity() * uiBase.getFontSize());
        for (auto &c: {&freqC, &gainC, &qC}) {
            c->getSlider1().setSliderStyle(style);
            c->getSlider1().setMouseDragSensitivity(sensitivity);
            c->getSlider2().setSliderStyle(style);
            c->getSlider2().setMouseDragSensitivity(sensitivity);
        }
    }

    void LeftControlPanel::resized() {
        juce::Grid grid;
        using Track = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;

        grid.templateRows = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1))};
        grid.templateColumns = {
            Track(Fr(30)), Track(Fr(60)),
            Track(Fr(60)), Track(Fr(60)), Track(Fr(60)), Track(Fr(30)), Track(Fr(30))
        };
        grid.items = {
            juce::GridItem(bypassC).withArea(1, 1, 4, 2),
            juce::GridItem(fTypeC).withArea(1, 2, 3, 3),
            juce::GridItem(freqC).withArea(1, 3, 7, 4),
            juce::GridItem(gainC).withArea(1, 4, 7, 5),
            juce::GridItem(qC).withArea(1, 5, 7, 6),
            juce::GridItem(soloC).withArea(4, 1, 7, 2),
            juce::GridItem(slopeC).withArea(3, 2, 5, 3),
            juce::GridItem(stereoC).withArea(5, 2, 7, 3),
            juce::GridItem(lrBox).withArea(2, 6, 4, 8),
            juce::GridItem(dynONC).withArea(4, 6, 7, 7),
            juce::GridItem(dynLC).withArea(4, 7, 7, 8),
        };

        for (auto &s: {&freqC, &gainC, &qC}) {
            s->setPadding(uiBase.getFontSize() * 0.5f, 0.f);
        }
        lrBox.setPadding(uiBase.getFontSize() * 2.f, 0.f);

        auto bound = getLocalBounds().toFloat();
        bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {});
        grid.performLayout(bound.toNearestInt());

        const auto resetBound = juce::Rectangle<float>(bound.getTopRight().getX() - 1.25f * uiBase.getFontSize(),
                                                       bound.getTopRight().getY(),
                                                       1.25f * uiBase.getFontSize(), 1.25f * uiBase.getFontSize());
        resetComponent.setBounds(resetBound.toNearestInt());
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
        parameterChanged(zlDSP::fType::ID + suffix,
                         parametersRef.getRawParameterValue(zlDSP::fType::ID + suffix)->load());
        parameterChanged(zlDSP::dynamicON::ID + suffix,
                         parametersRef.getRawParameterValue(zlDSP::dynamicON::ID + suffix)->load());
    }

    void LeftControlPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        if (parameterID.startsWith(zlDSP::fType::ID)) {
            switch (static_cast<zlIIR::FilterType>(newValue)) {
                case zlIIR::FilterType::peak:
                case zlIIR::FilterType::lowShelf:
                case zlIIR::FilterType::highShelf:
                case zlIIR::FilterType::bandShelf:
                case zlIIR::FilterType::tiltShelf: {
                    gainCEditable.store(true);
                    break;
                }
                case zlIIR::FilterType::lowPass:
                case zlIIR::FilterType::highPass:
                case zlIIR::FilterType::bandPass:
                case zlIIR::FilterType::notch: {
                    gainCEditable.store(false);
                    break;
                }
            }
            switch (static_cast<zlIIR::FilterType>(newValue)) {
                case zlIIR::FilterType::lowPass:
                case zlIIR::FilterType::highPass:
                case zlIIR::FilterType::lowShelf:
                case zlIIR::FilterType::highShelf:
                case zlIIR::FilterType::bandShelf:
                case zlIIR::FilterType::tiltShelf: {
                    slopCEnable.store(true);
                    break;
                }
                case zlIIR::FilterType::peak:
                case zlIIR::FilterType::bandPass:
                case zlIIR::FilterType::notch: {
                    slopCEnable.store(false);
                    break;
                }
            }
            if (idx == bandIdx.load()) {
                triggerAsyncUpdate();
            }
        } else if (parameterID.startsWith(zlDSP::dynamicON::ID)) {
            const auto f = static_cast<bool>(newValue); {
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
}
