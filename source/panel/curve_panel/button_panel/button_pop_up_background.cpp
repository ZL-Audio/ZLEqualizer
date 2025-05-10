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
        : band_idx_{bandIdx}, parameters_ref_(parameters), parameters_NA_ref_(parameters_NA),
          ui_base_(base),
          bypass_c_("B", base),
          solo_c_("S", base),
          bypass_drawable_(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg, BinaryData::fadpowerswitch_svgSize)),
          solo_drawable_(juce::Drawable::createFromImageData(BinaryData::fadsolo_svg, BinaryData::fadsolo_svgSize)),
          ftype_c_("", zlp::fType::choices, base),
          close_drawable_(juce::Drawable::createFromImageData(BinaryData::xmark_svg, BinaryData::xmark_svgSize)),
          close_c_(base, close_drawable_.get()) {
        juce::ignoreUnused(parameters_NA_ref_);
        bypass_c_.getLAF().enableShadow(false);
        bypass_c_.getLAF().setReverse(true);
        solo_c_.getLAF().enableShadow(false);
        bypass_c_.setDrawable(bypass_drawable_.get());
        solo_c_.setDrawable(solo_drawable_.get());
        for (auto &c: {&bypass_c_, &solo_c_}) {
            addAndMakeVisible(c);
        }
        attach({&bypass_c_.getButton(), &solo_c_.getButton()},
               {
                   zlp::appendSuffix(zlp::bypass::ID, bandIdx),
                   zlp::appendSuffix(zlp::solo::ID, bandIdx)
               },
               parameters_ref_, button_attachments_);

        bypass_c_.getButton().onClick = [this]() {
            const auto isByPassed = static_cast<float>(bypass_c_.getButton().getToggleState());
            const auto currentBand = band_idx_;
            const auto isCurrentBandSelected = ui_base_.getIsBandSelected(currentBand);
            for(size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
                if (idx == currentBand || (isCurrentBandSelected && ui_base_.getIsBandSelected(idx))) {
                    const auto activeID = zlstate::appendSuffix(zlp::bypass::ID, idx);
                    parameters_ref_.getParameter(activeID)->beginChangeGesture();
                    parameters_ref_.getParameter(activeID)->setValueNotifyingHost(isByPassed);
                    parameters_ref_.getParameter(activeID)->endChangeGesture();
                }
            }
        };

        ftype_c_.getLAF().setFontScale(1.25f);
        for (auto &c: {&ftype_c_}) {
            addAndMakeVisible(c);
        }
        attach({&ftype_c_.getBox()},
               {zlp::appendSuffix(zlp::fType::ID, bandIdx)},
               parameters_ref_, box_attachments_);

        close_c_.getButton().onClick = [this]() {
            const auto currentBand = band_idx_;
            const auto isCurrentBandSelected = ui_base_.getIsBandSelected(currentBand);
            for(size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
                if (idx == currentBand || (isCurrentBandSelected && ui_base_.getIsBandSelected(idx))) {
                    const auto activeID = zlstate::appendSuffix(zlstate::active::ID, idx);
                    parameters_NA_ref_.getParameter(activeID)->beginChangeGesture();
                    parameters_NA_ref_.getParameter(activeID)->setValueNotifyingHost(static_cast<float>(false));
                    parameters_NA_ref_.getParameter(activeID)->endChangeGesture();
                }
            }
        };
        close_c_.setPadding(.05f, .05f, .05f, .05f);
        addAndMakeVisible(close_c_);

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
            juce::GridItem(bypass_c_).withArea(1, 1),
            juce::GridItem(solo_c_).withArea(1, 2),
            juce::GridItem(close_c_).withArea(2, 3, 3, 4),
            juce::GridItem(ftype_c_).withArea(2, 1, 3, 3)
        };

        const auto bound = getLocalBounds().toFloat();
        grid.performLayout(bound.toNearestInt());
    }
} // zlpanel
