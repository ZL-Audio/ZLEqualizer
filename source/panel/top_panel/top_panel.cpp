// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "top_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    TopPanel::TopPanel(PluginProcessor& p, zlgui::UIBase& base,
                       multilingual::TooltipHelper& tooltip_helper) :
        base_(base), updater_(),
        logo_panel_(p, base, tooltip_helper),
        fstruct_box_(zlp::PFilterStructure::kChoices, base,
                     tooltip_helper.getToolTipText(multilingual::kFilterStructure),
                     {tooltip_helper.getToolTipText(multilingual::kMinimumPhase),
                      tooltip_helper.getToolTipText(multilingual::kStateVariable),
                      tooltip_helper.getToolTipText(multilingual::kParallelPhase),
                      tooltip_helper.getToolTipText(multilingual::kMatchedPhase),
                      tooltip_helper.getToolTipText(multilingual::kMixedPhase),
                      tooltip_helper.getToolTipText(multilingual::kLinearPhase)}),
        fstruct_attach_(fstruct_box_.getBox(), p.parameters_, zlp::PFilterStructure::kID, updater_),
        bypass_drawable_(juce::Drawable::createFromImageData(BinaryData::bypass_svg,
                                                             BinaryData::bypass_svgSize)),
        bypass_button_(base, bypass_drawable_.get(), bypass_drawable_.get(),
                       tooltip_helper.getToolTipText(multilingual::kBypass)),
        bypass_attach_(bypass_button_.getButton(), p.parameters_, zlp::PBypass::kID, updater_),
        ext_drawable_(juce::Drawable::createFromImageData(BinaryData::externalside_svg,
                                                          BinaryData::externalside_svgSize)),
        ext_button_(base, ext_drawable_.get(), ext_drawable_.get(),
                    tooltip_helper.getToolTipText(multilingual::kExternalSideChain)),
        ext_attach_(ext_button_.getButton(), p.parameters_, zlp::PExtSide::kID, updater_) {
        logo_panel_.setBufferedToImage(true);
        addAndMakeVisible(logo_panel_);

        fstruct_box_.setBufferedToImage(true);
        addAndMakeVisible(fstruct_box_);

        bypass_button_.setImageAlpha(1.f, 1.f, .5f, .75f);
        bypass_button_.setBufferedToImage(true);
        addAndMakeVisible(bypass_button_);

        ext_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        ext_button_.setBufferedToImage(true);
        addAndMakeVisible(ext_button_);

        setInterceptsMouseClicks(false, true);
    }

    void TopPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColour());
    }

    void TopPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);
        const auto small_slider_width = getSmallSliderWidth(font_size);
        auto bound = getLocalBounds();
        bound.removeFromTop(padding);
        bound.removeFromLeft(padding);
        bound.removeFromRight(padding / 2);
        const auto spacing = (slider_width - 2 * bound.getHeight()) / 2;

        logo_panel_.setBounds(bound.removeFromLeft(bound.getHeight() * 2 + padding));
        bound.removeFromLeft(spacing);
        fstruct_box_.setBounds(bound.removeFromLeft(small_slider_width * 2));

        bypass_button_.setBounds(bound.removeFromRight(bound.getHeight()));
        bound.removeFromRight(padding);
        ext_button_.setBounds(bound.removeFromRight(bound.getHeight()));
    }

    void TopPanel::repaintCallbackSlow() {
        updater_.updateComponents();
    }
}
