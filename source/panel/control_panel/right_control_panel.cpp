// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "right_control_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    RightControlPanel::RightControlPanel(PluginProcessor& p,
                                         zlgui::UIBase& base,
                                         const multilingual::TooltipHelper& tooltip_helper) :
        p_ref_(p), base_(base), updater_(),
        control_background_(base),
        bypass_drawable_(juce::Drawable::createFromImageData(BinaryData::bypass_svg,
                                                             BinaryData::bypass_svgSize)),
        bypass_button_(base, bypass_drawable_.get(), bypass_drawable_.get(),
                       tooltip_helper.getToolTipText(multilingual::kBandDynamicBypass)),
        auto_drawable_(juce::Drawable::createFromImageData(BinaryData::circle_a_svg,
                                                           BinaryData::circle_a_svgSize)),
        auto_button_(base, auto_drawable_.get(), auto_drawable_.get(),
                     tooltip_helper.getToolTipText(multilingual::kBandDynamicAuto)),
        relative_drawable_(juce::Drawable::createFromImageData(BinaryData::circle_r_svg,
                                                               BinaryData::circle_r_svgSize)),
        relative_button_(base, relative_drawable_.get(), relative_drawable_.get(),
                         tooltip_helper.getToolTipText(multilingual::kBandDynamicRelative)),
        swap_drawable_(juce::Drawable::createFromImageData(BinaryData::shuffle_svg,
                                                           BinaryData::shuffle_svgSize)),
        swap_button_(base, swap_drawable_.get(), swap_drawable_.get(),
                     tooltip_helper.getToolTipText(multilingual::kBandSideSwap)),
        link_drawable_(juce::Drawable::createFromImageData(BinaryData::link_svg,
                                                           BinaryData::link_svgSize)),
        link_button_(base, link_drawable_.get(), link_drawable_.get()),
        ftype_box_(zlp::PSideFilterType::kChoices, base),
        slope_box_(juce::StringArray{
                       "6 dB/oct", "12 dB/oct", "24 dB/oct", "36 dB/oct",
                       "48 dB/oct", "72 dB/oct", "96 dB/oct"
                   }, base),
        th_slider_("Threshold", base,
                   tooltip_helper.getToolTipText(multilingual::kBandDynamicThreshold)),
        knee_slider_("Knee", base,
                     tooltip_helper.getToolTipText(multilingual::kBandDynamicThreshold)),
        attack_slider_("Attack", base,
                       tooltip_helper.getToolTipText(multilingual::kBandDynamicAttack)),
        release_slider_("Release", base,
                        tooltip_helper.getToolTipText(multilingual::kBandDynamicRelease)),
        label_laf_(base),
        freq_label_("", "FREQ"),
        q_label_("", "Q"),
        freq_slider_("", base,
                     tooltip_helper.getToolTipText(multilingual::kBandDynamicSideFreq)),
        q_slider_("", base,
                  tooltip_helper.getToolTipText(multilingual::kBandDynamicSideQ)) {
        control_background_.setBufferedToImage(true);
        addAndMakeVisible(control_background_);

        bypass_button_.setImageAlpha(1.f, 1.f, .5f, .75f);
        bypass_button_.setBufferedToImage(true);
        addAndMakeVisible(bypass_button_);

        auto_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        auto_button_.setBufferedToImage(true);
        addAndMakeVisible(auto_button_);
        auto_button_.getButton().onClick = [this]() {
            if (base_.getSelectedBand() < zlp::kBandNum) {
                turnOnOffAuto();
            }
        };

        relative_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        relative_button_.setBufferedToImage(true);
        addAndMakeVisible(relative_button_);

        swap_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        swap_button_.setBufferedToImage(true);
        addAndMakeVisible(swap_button_);

        link_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        link_button_.setBufferedToImage(true);
        addAndMakeVisible(link_button_);

        const auto popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::upwards);
        slope_box_.getLAF().setItemJustification(juce::Justification::centredRight);
        for (auto& box : {&ftype_box_, &slope_box_}) {
            box->getLAF().setOption(popup_option);
            box->setBufferedToImage(true);
            addAndMakeVisible(box);
        }

        th_slider_.setBufferedToImage(true);
        addAndMakeVisible(th_slider_);

        knee_slider_.setBufferedToImage(true);
        addAndMakeVisible(knee_slider_);

        attack_slider_.setBufferedToImage(true);
        addAndMakeVisible(attack_slider_);

        release_slider_.setBufferedToImage(true);
        addAndMakeVisible(release_slider_);

        label_laf_.setFontScale(1.5f);
        for (auto& l : {&freq_label_, &q_label_}) {
            l->setLookAndFeel(&label_laf_);
            l->setJustificationType(juce::Justification::centred);
            l->setInterceptsMouseClicks(false, false);
            l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        freq_slider_.setPrecision(3);
        freq_slider_.permitted_characters_ = "-0123456789.kK#ABCDEFG";
        freq_slider_.string_formatter_ = freq_note::getFrequencyFromNote;
        freq_slider_.setBufferedToImage(true);
        addAndMakeVisible(freq_slider_);

        q_slider_.setPrecision(3);
        q_slider_.setBufferedToImage(true);
        addAndMakeVisible(q_slider_);

        ftype_box_.addMouseListener(this, false);
        freq_slider_.addMouseListener(this, false);
        q_slider_.addMouseListener(this, false);
        setInterceptsMouseClicks(false, true);
    }

    RightControlPanel::~RightControlPanel() = default;

    int RightControlPanel::getIdealWidth() const {
        const auto font_size = base_.getFontSize();
        const auto slider_width = getSliderWidth(font_size);
        const auto small_slider_width = getSmallSliderWidth(font_size);
        const auto padding = getPaddingSize(font_size);

        return 5 * padding + 2 * (padding / 2) + 2 * slider_width + 2 * small_slider_width;
    }

    void RightControlPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto slider_width = getSliderWidth(font_size);
        const auto slider_height = getSliderHeight(font_size);
        const auto button_height = getButtonSize(font_size);
        const auto small_slider_width = getSmallSliderWidth(font_size);
        const auto label_height = juce::roundToInt(base_.getFontSize() * 1.5f) + 1;
        const auto padding = getPaddingSize(font_size);

        auto bound = getLocalBounds();
        control_background_.setBounds(bound);

        bound.reduce(padding + padding / 2, padding);

        auto top_bound = bound.removeFromTop(button_height);
        const auto w_padding = (2 * slider_width + padding - 4 * button_height) / 4;
        bypass_button_.setBounds(top_bound.removeFromLeft(button_height));
        top_bound.removeFromLeft(w_padding);
        auto_button_.setBounds(top_bound.removeFromLeft(button_height));
        top_bound.removeFromLeft(w_padding);
        relative_button_.setBounds(top_bound.removeFromLeft(button_height));
        top_bound.removeFromLeft(w_padding);
        swap_button_.setBounds(top_bound.removeFromLeft(button_height));
        top_bound.removeFromLeft(2 * slider_width + 2 * padding - 4 * button_height - 3 * w_padding);
        link_button_.setBounds(top_bound.removeFromLeft(button_height));
        top_bound.removeFromLeft(padding);
        slope_box_.setBounds(top_bound.removeFromRight(slider_width));
        top_bound.removeFromRight(padding);
        ftype_box_.setBounds(top_bound);

        const auto h_padding = (bound.getHeight() - 2 * slider_height) / 4;
        {
            auto temp_bound = bound.removeFromLeft(slider_width);
            temp_bound.removeFromBottom(h_padding);
            knee_slider_.setBounds(temp_bound.removeFromBottom(slider_height));
            temp_bound.removeFromBottom(2 * h_padding);
            th_slider_.setBounds(temp_bound.removeFromBottom(slider_height));
        }
        bound.removeFromLeft(padding);
        {
            auto temp_bound = bound.removeFromLeft(slider_width);
            temp_bound.removeFromBottom(h_padding);
            release_slider_.setBounds(temp_bound.removeFromBottom(slider_height));
            temp_bound.removeFromBottom(2 * h_padding);
            attack_slider_.setBounds(temp_bound.removeFromBottom(slider_height));
        }
        bound.removeFromLeft(padding);
        {
            auto temp_bound = bound.removeFromLeft(small_slider_width);
            freq_label_.setBounds(temp_bound.removeFromTop(label_height));
            freq_slider_.setBounds(temp_bound);
        }
        bound.removeFromLeft(padding);
        {
            auto temp_bound = bound.removeFromLeft(small_slider_width);
            q_label_.setBounds(temp_bound.removeFromTop(label_height));
            q_slider_.setBounds(temp_bound);
        }
    }

    void RightControlPanel::repaintCallBackSlow() {
        updater_.updateComponents();
        if (c_side_ftype_ != ftype_box_.getBox().getSelectedItemIndex()) {
            c_side_ftype_ = ftype_box_.getBox().getSelectedItemIndex();
            if (c_side_ftype_ < 0) {
                return;
            }
            if (c_side_ftype_ == 0 && slope_box_.getBox().getSelectedId() == 1) {
                slope_box_.getBox().setSelectedId(2, juce::sendNotificationSync);
            }
            if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum && c_side_ftype_ != 0) {
                auto* para = p_ref_.parameters_.getParameter(zlp::PSideQ::kID + std::to_string(band));
                updateValue(para, para->getDefaultValue());
            }
            slope_box_.getBox().setItemEnabled(1, c_side_ftype_ != 0);
            q_slider_.setEditable(c_side_ftype_ == 0);
            q_label_.setAlpha(c_side_ftype_ == 0 ? 1.f : .5f);
        }
    }

    void RightControlPanel::updateBand() {
        if (base_.getSelectedBand() < zlp::kBandNum) {
            const auto band_s = std::to_string(base_.getSelectedBand());
            bypass_attachment_ = std::make_unique<zlgui::attachment::ButtonAttachment<true>>(
                bypass_button_.getButton(), p_ref_.parameters_, zlp::PDynamicBypass::kID + band_s, updater_);
            auto_attachment_ = std::make_unique<zlgui::attachment::ButtonAttachment<true>>(
                auto_button_.getButton(), p_ref_.parameters_, zlp::PDynamicLearn::kID + band_s, updater_,
                juce::dontSendNotification);
            relative_attachment_ = std::make_unique<zlgui::attachment::ButtonAttachment<true>>(
                relative_button_.getButton(), p_ref_.parameters_, zlp::PDynamicRelative::kID + band_s, updater_);
            swap_attachment_ = std::make_unique<zlgui::attachment::ButtonAttachment<true>>(
                swap_button_.getButton(), p_ref_.parameters_, zlp::PSideSwap::kID + band_s, updater_);
            link_attachment_ = std::make_unique<zlgui::attachment::ButtonAttachment<true>>(
                link_button_.getButton(), p_ref_.parameters_, zlp::PSideLink::kID + band_s, updater_);
            ftype_attachment_ = std::make_unique<zlgui::attachment::ComboBoxAttachment<true>>(
                ftype_box_.getBox(), p_ref_.parameters_, zlp::PSideFilterType::kID + band_s, updater_);
            slope_attachment_ = std::make_unique<zlgui::attachment::ComboBoxAttachment<true>>(
                slope_box_.getBox(), p_ref_.parameters_, zlp::PSideOrder::kID + band_s, updater_);
            th_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                th_slider_.getSlider(), p_ref_.parameters_, zlp::PThreshold::kID + band_s, updater_);
            knee_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                knee_slider_.getSlider(), p_ref_.parameters_, zlp::PKneeW::kID + band_s, updater_);
            attack_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                attack_slider_.getSlider(), p_ref_.parameters_, zlp::PAttack::kID + band_s, updater_);
            release_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                release_slider_.getSlider(), p_ref_.parameters_, zlp::PRelease::kID + band_s, updater_);
            updateFreqMax(freq_max_);
            q_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                q_slider_.getSlider1(), p_ref_.parameters_, zlp::PSideQ::kID + band_s, updater_);
        } else {
            bypass_attachment_.reset();
            auto_attachment_.reset();
            relative_attachment_.reset();
            swap_attachment_.reset();
            ftype_attachment_.reset();
            slope_attachment_.reset();
            th_attachment_.reset();
            knee_attachment_.reset();
            attack_attachment_.reset();
            release_attachment_.reset();
            freq_attachment_.reset();
            q_attachment_.reset();
        }
    }

    void RightControlPanel::updateFreqMax(const double freq_max) {
        freq_max_ = freq_max;
        if (base_.getSelectedBand() < zlp::kBandNum) {
            freq_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                freq_slider_.getSlider1(), p_ref_.parameters_,
                zlp::PSideFreq::kID + std::to_string(base_.getSelectedBand()),
                zlp::getLogMidRange(10.0, freq_max, 1000.0, 0.1),
                updater_);
            freq_slider_.visibilityChanged();
        } else {
            freq_attachment_.reset();
        }
    }

    void RightControlPanel::turnOnOffAuto() {
        const auto band = base_.getSelectedBand();
        if (!auto_button_.getButton().getToggleState()) {
            auto* th_para = p_ref_.parameters_.getParameter(zlp::PThreshold::kID + std::to_string(band));
            updateValue(th_para,
                        th_para->convertTo0to1(static_cast<float>(p_ref_.getController().getLearnedThreshold(band))));
            auto* knee_para = p_ref_.parameters_.getParameter(zlp::PKneeW::kID + std::to_string(band));
            updateValue(knee_para,
                        knee_para->convertTo0to1(static_cast<float>(p_ref_.getController().getLearnedKnee(band))));
        } else {
            auto* th_para = p_ref_.parameters_.getParameter(zlp::PThreshold::kID + std::to_string(band));
            updateValue(th_para, th_para->getDefaultValue());
        }
    }

    void RightControlPanel::mouseDown(const juce::MouseEvent&) {
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            updateValue(p_ref_.parameters_.getParameter(zlp::PSideLink::kID + std::to_string(band)), 0.f);
        }
    }

    void RightControlPanel::mouseDrag(const juce::MouseEvent& event) {
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            if (event.originalComponent == &freq_slider_ || event.originalComponent == &q_slider_) {
                updateValue(p_ref_.parameters_.getParameter(zlp::PSideLink::kID + std::to_string(band)), 0.f);
            }
        }
    }

    void RightControlPanel::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails&) {
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            if (event.originalComponent == &freq_slider_ || event.originalComponent == &q_slider_) {
                updateValue(p_ref_.parameters_.getParameter(zlp::PSideLink::kID + std::to_string(band)), 0.f);
            }
        }
    }
}
