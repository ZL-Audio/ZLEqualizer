// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "output_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    OutputPanel::OutputPanel(PluginProcessor& p, zlgui::UIState& base,
                             const multilingual::TooltipHelper& tooltip_helper) :
        p_ref_(p), base_(base), updater_(),
        control_background_(base),
        name_laf_(base.gui),
        gain_label_("", "GAIN"),
        scale_label_("", "SCALE"),
        gain_slider_("", base_.gui,
                     tooltip_helper.getToolTipText(multilingual::kOutputGain), 1.25f),
        gain_attach_(gain_slider_.getSlider1(), p.parameters_,
                     zlp::POutputGain::kID, updater_),
        scale_slider_("", base.gui,
                      tooltip_helper.getToolTipText(multilingual::kGainScale), 1.25f),
        scale_attach_(scale_slider_.getSlider1(), p.parameters_,
                      zlp::PGainScale::kID, updater_),
        sgc_button_(base.gui,
                    juce::Drawable::createFromImageData(BinaryData::dline_s_svg, BinaryData::dline_s_svgSize),
                    true, tooltip_helper.getToolTipText(multilingual::kStaticGC)),
        sgc_attach_(sgc_button_.getButton(), p.parameters_,
                    zlp::PStaticGain::kID, updater_),
        lm_button_(base.gui,
                   juce::Drawable::createFromImageData(BinaryData::dline_l_svg, BinaryData::dline_l_svgSize),
                   true, tooltip_helper.getToolTipText(multilingual::kLoudnessGC)),
        agc_button_(base.gui,
                    juce::Drawable::createFromImageData(BinaryData::dline_a_svg, BinaryData::dline_a_svgSize),
                    true, tooltip_helper.getToolTipText(multilingual::kAutoGC)),
        agc_attach_(agc_button_.getButton(), p.parameters_,
                    zlp::PAutoGain::kID, updater_),
        phase_button_(base.gui,
                      juce::Drawable::createFromImageData(BinaryData::phase_svg, BinaryData::phase_svgSize),
                      true, tooltip_helper.getToolTipText(multilingual::kPhaseFlip)),
        phase_attach_(phase_button_.getButton(), p.parameters_,
                      zlp::PPhaseFlip::kID, updater_),
        stereo_box_([]() -> std::vector<std::unique_ptr<juce::Drawable>> {
            std::vector<std::unique_ptr<juce::Drawable>> icons;
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::stereo_svg, BinaryData::stereo_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::left_svg, BinaryData::left_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::right_svg, BinaryData::right_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::mid_svg, BinaryData::mid_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::side_svg, BinaryData::side_svgSize));
            return icons;
        }(), base.gui, tooltip_helper.getToolTipText(multilingual::kBandStereoMode),
                    {"Stereo", "Left", "Right", "Mid", "Side"}),
        stereo_attach_(stereo_box_.getBox(), p.parameters_,
                       zlp::POutputStereo::kID, updater_),
        lookahead_label_("", "Lookahead"),
        lookahead_slider_("", base.gui,
                          tooltip_helper.getToolTipText(multilingual::kLookahead)),
        lookahead_attach_(lookahead_slider_.getSlider(), p.parameters_,
                          zlp::PLookahead::kID, updater_) {
        juce::ignoreUnused(p_ref_, base_, tooltip_helper);

        control_background_.setBufferedToImage(true);
        addAndMakeVisible(control_background_);

        name_laf_.setFontScale(1.5f);

        gain_label_.setLookAndFeel(&name_laf_);
        gain_label_.setJustificationType(juce::Justification::centred);
        gain_label_.setBufferedToImage(true);
        addAndMakeVisible(gain_label_);

        scale_label_.setLookAndFeel(&name_laf_);
        scale_label_.setJustificationType(juce::Justification::centred);
        scale_label_.setBufferedToImage(true);
        addAndMakeVisible(scale_label_);

        gain_slider_.setComponentID(zlp::POutputGain::kID);
        gain_slider_.setBufferedToImage(true);
        addAndMakeVisible(gain_slider_);

        scale_slider_.setComponentID(zlp::PGainScale::kID);
        scale_slider_.setBufferedToImage(true);
        addAndMakeVisible(scale_slider_);

        lm_button_.getButton().onClick = [this]() {
            if (lm_button_.getToggleState()) {
                p_ref_.getController().setLoudnessMatchON(true);
            } else {
                p_ref_.getController().setLoudnessMatchON(false);
                const auto c_diff = static_cast<float>(p_ref_.getController().getLUFSMatcherDiff());
                auto* output_gain_para = p_ref_.parameters_.getParameter(zlp::POutputGain::kID);
                updateValue(output_gain_para, output_gain_para->convertTo0to1(-c_diff));
                auto* agc_para = p_ref_.parameters_.getParameter(zlp::PAutoGain::kID);
                updateValue(agc_para, 0.f);
            }
        };

        for (auto& b : {&sgc_button_, &lm_button_, &agc_button_, &phase_button_}) {
            b->setImageAlpha(.5f, .75f, 1.f, 1.f);
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }

        stereo_box_.setComponentID(zlp::POutputStereo::kID);
        stereo_box_.getLAF().setOption(juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::downwards));
        stereo_box_.setBufferedToImage(true);
        addAndMakeVisible(stereo_box_);

        lookahead_label_.setLookAndFeel(&name_laf_);
        lookahead_label_.setJustificationType(juce::Justification::centredRight);
        lookahead_label_.setBufferedToImage(true);
        addAndMakeVisible(lookahead_label_);

        lookahead_slider_.getSlider().setSliderSnapsToMousePosition(false);
        lookahead_slider_.setBufferedToImage(true);
        addAndMakeVisible(lookahead_slider_);

        base_.setPanelProperty(zlgui::PanelSettingIdx::kOutputPanel, 0.);
        base_.getPanelValueTree().addListener(this);
    }

    OutputPanel::~OutputPanel() {
        base_.getPanelValueTree().removeListener(this);
        p_ref_.getController().setLoudnessMatchON(false);
    }

    int OutputPanel::getIdealWidth() const {
        const auto font_size = base_.gui.getFontSize();
        const auto slider_width = getSliderWidth(font_size);
        const auto padding = getPaddingSize(font_size);
        return 5 * padding + 2 * slider_width;
    }

    int OutputPanel::getIdealHeight() const {
        const auto font_size = base_.gui.getFontSize();
        const auto slider_width = getSliderWidth(font_size);
        const auto button_height = getButtonSize(font_size);
        const auto padding = getPaddingSize(font_size);
        return 5 * padding + 3 * button_height + slider_width;
    }

    void OutputPanel::resized() {
        const auto font_size = base_.gui.getFontSize();
        const auto slider_width = getSliderWidth(font_size);
        const auto button_height = getButtonSize(font_size);
        const auto padding = getPaddingSize(font_size);

        auto bound = getLocalBounds();
        control_background_.setBounds(bound);

        bound.reduce(2 * padding, padding);
        {
            auto t_bound = bound.removeFromTop(button_height);
            gain_label_.setBounds(t_bound.removeFromLeft(slider_width));
            scale_label_.setBounds(t_bound.removeFromRight(slider_width));
        }
        bound.removeFromTop(padding);
        {
            auto t_bound = bound.removeFromTop(slider_width);
            gain_slider_.setBounds(t_bound.removeFromLeft(slider_width));
            scale_slider_.setBounds(t_bound.removeFromRight(slider_width));
        }
        bound.removeFromTop(padding);
        {
            const auto row = bound.removeFromTop(button_height);
            const auto step = static_cast<float>(row.getWidth() - button_height) / 4.f;
            const auto get_control_bounds = [row, step, button_height](const int index) {
                return row.withWidth(button_height).translated(
                    static_cast<int>(std::round(static_cast<float>(index) * step)), 0);
            };
            sgc_button_.setBounds(get_control_bounds(0));
            lm_button_.setBounds(get_control_bounds(1));
            agc_button_.setBounds(get_control_bounds(2));
            phase_button_.setBounds(get_control_bounds(3));
            const auto stereo_box_bound = get_control_bounds(4).reduced(
                static_cast<int>(std::round(font_size * .1f)));

            stereo_box_.getLAF().setIconSize(static_cast<float>(stereo_box_bound.getHeight()));
            stereo_box_.setBounds(stereo_box_bound.expanded(padding, 0));
        }
        bound.removeFromTop(padding);
        {
            auto t_bound = bound.removeFromTop(button_height);
            lookahead_slider_.setBounds(t_bound.removeFromRight(t_bound.getWidth() / 3));
            t_bound.removeFromRight(padding);
            lookahead_label_.setBounds(t_bound);
        }

        const auto dragging_distance = getSliderDraggingDistance(font_size);
        gain_slider_.setMouseDragSensitivity(dragging_distance);
        scale_slider_.setMouseDragSensitivity(dragging_distance);
        lookahead_slider_.setMouseDragSensitivity(dragging_distance);
    }

    void OutputPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }

    void OutputPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kOutputPanel, property)) {
            setVisible(static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kOutputPanel)) > .5);
        }
    }
}
