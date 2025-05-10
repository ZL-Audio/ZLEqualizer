// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "button_pop_up_background.hpp"

namespace zlpanel {
    ButtonPopUpBackground::ButtonPopUpBackground(size_t bandIdx, juce::AudioProcessorValueTreeState &parameters, juce::AudioProcessorValueTreeState &parameters_NA, zlgui::UIBase &base)
        : band{bandIdx}, parameters_ref_(parameters), parameters_NA_ref_(parameters_NA),
          ui_base_(base),
          bypassC("B", base),
          soloC("S", base),
          bypassDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg, BinaryData::fadpowerswitch_svgSize)),
          soloDrawable(juce::Drawable::createFromImageData(BinaryData::fadsolo_svg, BinaryData::fadsolo_svgSize)),
          fTypeC("", zlp::fType::choices, base),
          drawable(juce::Drawable::createFromImageData(BinaryData::xmark_svg, BinaryData::xmark_svgSize)),
          button(base, drawable.get()) {
        juce::ignoreUnused(parameters_NA_ref_);
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
                   zlp::appendSuffix(zlp::bypass::ID, bandIdx),
                   zlp::appendSuffix(zlp::solo::ID, bandIdx)
               },
               parameters_ref_, buttonAttachments);

        bypassC.getButton().onClick = [this]() {
            const auto isByPassed = static_cast<float>(bypassC.getButton().getToggleState());
            const auto currentBand = band;
            const auto isCurrentBandSelected = ui_base_.getIsBandSelected(currentBand);
            for(size_t idx = 0; idx < zlstate::bandNUM; ++idx) {
                if (idx == currentBand || (isCurrentBandSelected && ui_base_.getIsBandSelected(idx))) {
                    const auto activeID = zlstate::appendSuffix(zlp::bypass::ID, idx);
                    parameters_ref_.getParameter(activeID)->beginChangeGesture();
                    parameters_ref_.getParameter(activeID)->setValueNotifyingHost(isByPassed);
                    parameters_ref_.getParameter(activeID)->endChangeGesture();
                }
            }
        };

        fTypeC.getLAF().setFontScale(1.25f);
        for (auto &c: {&fTypeC}) {
            addAndMakeVisible(c);
        }
        attach({&fTypeC.getBox()},
               {zlp::appendSuffix(zlp::fType::ID, bandIdx)},
               parameters_ref_, boxAttachments);

        button.getButton().onClick = [this]() {
            const auto currentBand = band;
            const auto isCurrentBandSelected = ui_base_.getIsBandSelected(currentBand);
            for(size_t idx = 0; idx < zlstate::bandNUM; ++idx) {
                if (idx == currentBand || (isCurrentBandSelected && ui_base_.getIsBandSelected(idx))) {
                    const auto activeID = zlstate::appendSuffix(zlstate::active::ID, idx);
                    parameters_NA_ref_.getParameter(activeID)->beginChangeGesture();
                    parameters_NA_ref_.getParameter(activeID)->setValueNotifyingHost(static_cast<float>(false));
                    parameters_NA_ref_.getParameter(activeID)->endChangeGesture();
                }
            }
        };
        button.setPadding(.05f, .05f, .05f, .05f);
        addAndMakeVisible(button);

        setBufferedToImage(true);
    }

    void ButtonPopUpBackground::paint(juce::Graphics &g) {
        g.setColour(ui_base_.getBackgroundColor().withMultipliedAlpha(.25f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), ui_base_.getFontSize() * .5f);
        g.setColour(ui_base_.getTextColor().withMultipliedAlpha(.25f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), ui_base_.getFontSize() * .5f);
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
} // zlpanel
