// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "analyzer_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    AnalyzerPanel::AnalyzerPanel(PluginProcessor& p, zlgui::UIBase& base,
                                 const multilingual::TooltipHelper& tooltip_helper) :
        base_(base),
        control_background_(base),
        pre_button_(base, "Pre",
                    tooltip_helper.getToolTipText(multilingual::kFFTPre)),
        pre_attach_(pre_button_.getButton(), p.parameters_NA_,
                    zlstate::PFFTPreON::kID, updater_),
        post_button_(base, "Post",
                     tooltip_helper.getToolTipText(multilingual::kFFTPost)),
        post_attach_(post_button_.getButton(), p.parameters_NA_,
                     zlstate::PFFTPostON::kID, updater_),
        side_button_(base, "Side",
                     tooltip_helper.getToolTipText(multilingual::kFFTSide)),
        side_attach_(side_button_.getButton(), p.parameters_NA_,
                     zlstate::PFFTSideON::kID, updater_),
        speed_box_(zlstate::PFFTSpeed::kChoices, base,
                   tooltip_helper.getToolTipText(multilingual::kFFTDecay)),
        speed_attach_(speed_box_.getBox(), p.parameters_NA_,
                      zlstate::PFFTSpeed::kID, updater_),
        slope_box_(zlstate::PFFTTilt::kChoices, base,
                   tooltip_helper.getToolTipText(multilingual::kFFTSlope)),
        slope_attach_(slope_box_.getBox(), p.parameters_NA_,
                      zlstate::PFFTTilt::kID, updater_),
        freeze_drawable_(juce::Drawable::createFromImageData(BinaryData::freeze_svg,
                                                             BinaryData::freeze_svgSize)),
        freeze_button_(base, freeze_drawable_.get(), freeze_drawable_.get()),
        freeze_attach_(freeze_button_.getButton(), p.parameters_NA_,
                       zlstate::PFFTFreezeON::kID, updater_),
        collision_drawable_(juce::Drawable::createFromImageData(BinaryData::collision_svg,
                                                                BinaryData::collision_svgSize)),
        collision_button_(base, collision_drawable_.get(), collision_drawable_.get()),
        collision_attach_(collision_button_.getButton(), p.parameters_NA_,
                          zlstate::PCollisionON::kID, updater_),
        label_laf_(base),
        strength_label_("", "Strength"),
        strength_slider_("", base,
                         tooltip_helper.getToolTipText(multilingual::kCollisionStrength)),
        strength_attach_(strength_slider_.getSlider(), p.parameters_NA_,
                         zlstate::PCollisionStrength::kID, updater_) {

        control_background_.setBufferedToImage(true);
        addAndMakeVisible(control_background_);

        for (auto& b : {&pre_button_, &post_button_, &side_button_}) {
            b->getLAF().setFontScale(1.5f);
            b->getLAF().setJustification(juce::Justification::centred);
            b->getButton().setToggleable(true);
            b->getButton().setClickingTogglesState(true);
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }

        for (auto& c : {&speed_box_, &slope_box_}) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
        }

        for (auto& b : {&freeze_button_, &collision_button_}) {
            b->setImageAlpha(.5f, .5f, 1.f, 1.f);
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }

        label_laf_.setFontScale(1.5f);
        strength_label_.setLookAndFeel(&label_laf_);
        strength_label_.setBufferedToImage(true);
        addAndMakeVisible(strength_label_);

        strength_slider_.getSlider().setSliderSnapsToMousePosition(false);
        strength_slider_.setBufferedToImage(true);
        addAndMakeVisible(strength_slider_);

        base_.setPanelProperty(zlgui::PanelSettingIdx::kAnalyzerPanel, 0.);
        base_.getPanelValueTree().addListener(this);
    }

    AnalyzerPanel::~AnalyzerPanel() {
        base_.getPanelValueTree().removeListener(this);
    }

    int AnalyzerPanel::getIdealWidth() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);

        return 6 * padding + 3 * (slider_width / 2);
    }

    int AnalyzerPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto button_height = getButtonSize(font_size);

        return 6 * padding + 5 * button_height;
    }

    void AnalyzerPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto button_height = getButtonSize(font_size);
        const auto padding = getPaddingSize(font_size);

        auto bound = getLocalBounds();
        control_background_.setBounds(bound);

        bound.reduce(2 * padding, padding);
        {
            auto t_bound = bound.removeFromTop(button_height);
            const auto button_width = t_bound.getWidth() / 3;
            pre_button_.setBounds(t_bound.removeFromLeft(button_width));
            side_button_.setBounds(t_bound.removeFromRight(button_width));
            post_button_.setBounds(t_bound);
        }
        bound.removeFromTop(padding);
        speed_box_.setBounds(bound.removeFromTop(button_height));

        bound.removeFromTop(padding);
        slope_box_.setBounds(bound.removeFromTop(button_height));

        bound.removeFromTop(padding);
        {
            auto t_bound = bound.removeFromTop(button_height);
            const auto h_padding = (t_bound.getWidth() - button_height * 2) / 2;
            t_bound.removeFromLeft(h_padding / 2);
            freeze_button_.setBounds(t_bound.removeFromLeft(button_height));
            t_bound.removeFromRight(h_padding / 2);
            collision_button_.setBounds(t_bound.removeFromRight(button_height));
        }
        bound.removeFromTop(padding);
        {
            auto t_bound = bound.removeFromTop(button_height);
            strength_slider_.setBounds(t_bound.removeFromRight(t_bound.getWidth() / 3));
            strength_label_.setBounds(t_bound);
        }
    }

    void AnalyzerPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }

    void AnalyzerPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kAnalyzerPanel, property)) {
            setVisible(static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kAnalyzerPanel)) > .5);
        }
    }
}
