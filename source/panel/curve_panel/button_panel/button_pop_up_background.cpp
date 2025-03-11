// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "button_pop_up_background.hpp"

namespace zlPanel {
    ButtonPopUpBackground::ButtonPopUpBackground(size_t bandIdx, juce::AudioProcessorValueTreeState &parameters, juce::AudioProcessorValueTreeState &parametersNA, zlInterface::UIBase &base)
        : band{bandIdx}, parametersRef(parameters), parametersNARef(parametersNA),
          uiBase(base),
          bypassC("B", base),
          soloC("S", base),
          bypassDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg, BinaryData::fadpowerswitch_svgSize)),
          soloDrawable(juce::Drawable::createFromImageData(BinaryData::fadsolo_svg, BinaryData::fadsolo_svgSize)),
          fTypeC("", zlDSP::fType::choices, base),
          drawable(juce::Drawable::createFromImageData(BinaryData::xmark_svg, BinaryData::xmark_svgSize)),
          button(base, drawable.get()) {
        juce::ignoreUnused(parametersNARef);
        bypassC.getLAF().enableShadow(false);
        bypassC.getLAF().setReverse(true);
        soloC.getLAF().enableShadow(false);
        bypassC.setDrawable(bypassDrawable.get());
        soloC.setDrawable(soloDrawable.get());
        for (auto &c: {&bypassC, &soloC}) {
            addAndMakeVisible(c);
        }
        attach({&bypassC.getButton(), &soloC.getButton()},
               {
                   zlDSP::appendSuffix(zlDSP::bypass::ID, bandIdx),
                   zlDSP::appendSuffix(zlDSP::solo::ID, bandIdx)
               },
               parametersRef, buttonAttachments);

        bypassC.getButton().onClick = [this]() {
            const auto isByPassed = static_cast<float>(bypassC.getButton().getToggleState());
            const auto currentBand = band;
            const auto isCurrentBandSelected = uiBase.getIsBandSelected(currentBand);
            for(size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                if (idx == currentBand || (isCurrentBandSelected && uiBase.getIsBandSelected(idx))) {
                    const auto activeID = zlState::appendSuffix(zlDSP::bypass::ID, idx);
                    parametersRef.getParameter(activeID)->beginChangeGesture();
                    parametersRef.getParameter(activeID)->setValueNotifyingHost(isByPassed);
                    parametersRef.getParameter(activeID)->endChangeGesture();
                }
            }
        };

        fTypeC.getLAF().setFontScale(1.25f);
        for (auto &c: {&fTypeC}) {
            addAndMakeVisible(c);
        }
        attach({&fTypeC.getBox()},
               {zlDSP::appendSuffix(zlDSP::fType::ID, bandIdx)},
               parametersRef, boxAttachments);

        button.getButton().onClick = [this]() {
            const auto currentBand = band;
            const auto isCurrentBandSelected = uiBase.getIsBandSelected(currentBand);
            for(size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                if (idx == currentBand || (isCurrentBandSelected && uiBase.getIsBandSelected(idx))) {
                    const auto activeID = zlState::appendSuffix(zlState::active::ID, idx);
                    parametersNARef.getParameter(activeID)->beginChangeGesture();
                    parametersNARef.getParameter(activeID)->setValueNotifyingHost(static_cast<float>(false));
                    parametersNARef.getParameter(activeID)->endChangeGesture();
                }
            }
        };
        button.setPadding(.05f, .05f, .05f, .05f);
        addAndMakeVisible(button);

        setBufferedToImage(true);
    }

    void ButtonPopUpBackground::paint(juce::Graphics &g) {
        g.setColour(uiBase.getBackgroundColor().withMultipliedAlpha(.25f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), uiBase.getFontSize() * .5f);
        g.setColour(uiBase.getTextColor().withMultipliedAlpha(.25f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), uiBase.getFontSize() * .5f);
    }

    void ButtonPopUpBackground::resized() {
        juce::Grid grid;
        using Track = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;

        grid.templateRows = {Track(Fr(60)), Track(Fr(40))};
        grid.templateColumns = {
            Track(Fr(30)), Track(Fr(30)), Track(Fr(25))
        };
        grid.items = {
            juce::GridItem(bypassC).withArea(1, 1),
            juce::GridItem(soloC).withArea(1, 2),
            juce::GridItem(button).withArea(2, 3, 3, 4),
            juce::GridItem(fTypeC).withArea(2, 1, 3, 3)
        };

        const auto bound = getLocalBounds().toFloat();
        grid.performLayout(bound.toNearestInt());
    }
} // zlPanel
