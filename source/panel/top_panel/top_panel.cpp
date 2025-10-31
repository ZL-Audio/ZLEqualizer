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
        output_label_(p, base),
        analyzer_label_(p, base),
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
        ext_attach_(ext_button_.getButton(), p.parameters_, zlp::PExtSide::kID, updater_),
        match_drawable_(juce::Drawable::createFromImageData(BinaryData::match_svg,
                                                            BinaryData::match_svgSize)),
        match_button_(base, match_drawable_.get(), match_drawable_.get()) {
        logo_panel_.setBufferedToImage(true);
        addAndMakeVisible(logo_panel_);

        output_label_.setBufferedToImage(true);
        addAndMakeVisible(output_label_);

        analyzer_label_.setBufferedToImage(true);
        addAndMakeVisible(analyzer_label_);

        fstruct_box_.setAlpha(.5f);
        fstruct_box_.setBufferedToImage(true);
        addAndMakeVisible(fstruct_box_);

        bypass_button_.setImageAlpha(1.f, 1.f, .5f, .75f);
        bypass_button_.setBufferedToImage(true);
        addAndMakeVisible(bypass_button_);

        ext_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        ext_button_.setBufferedToImage(true);
        addAndMakeVisible(ext_button_);

        match_button_.getButton().onClick = [this]() {
            base_.setPanelProperty(zlgui::kMatchPanel, static_cast<double>(match_button_.getToggleState()));
        };
        match_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        match_button_.setBufferedToImage(true);
        addAndMakeVisible(match_button_);

        setInterceptsMouseClicks(false, true);
    }

    void TopPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColour());
    }

    int TopPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        return 2 * (getPaddingSize(font_size) / 2) + getButtonSize(font_size);
    }

    void TopPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);
        const auto small_slider_width = getSmallSliderWidth(font_size);
        auto bound = getLocalBounds();
        bound.reduce(padding / 2, padding / 2);

        logo_panel_.setBounds(bound.removeFromLeft(bound.getHeight() * 2 + padding));
        bound.removeFromLeft(padding);

        bypass_button_.setBounds(bound.removeFromRight(bound.getHeight()));
        bound.removeFromRight(padding);
        ext_button_.setBounds(bound.removeFromRight(bound.getHeight()));
        {
            const auto left_pad = bound.getX();
            const auto t_width = 6 * padding + 3 * (slider_width / 2) - left_pad;
            analyzer_label_.setBounds(bound.getX(), 0, t_width, getHeight());
            bound.removeFromLeft(t_width);
        }
        match_button_.setBounds(bound.removeFromLeft(bound.getHeight()));
        {
            const auto right_pad = getWidth() - bound.getRight();
            const auto t_width = 5 * padding + 2 * slider_width - right_pad + 2 * padding;
            output_label_.setBounds(bound.getRight() - t_width, 0, t_width, getHeight());
        }
        bound = getLocalBounds().reduced(0, padding / 2);
        fstruct_box_.setBounds(bound.withSizeKeepingCentre(small_slider_width * 2, bound.getHeight()));
    }

    void TopPanel::repaintCallbackSlow() {
        output_label_.repaintCallbackSlow();
        updater_.updateComponents();
    }
}
