// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "sub_left_control_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    SubLeftControlPanel::SubLeftControlPanel(PluginProcessor& p,
                                             zlgui::UIBase& base,
                                             const multilingual::TooltipHelper& tooltip_helper) :
        p_ref_(p), base_(base), updater_(),
        control_background_(base),
        close_drawable_(juce::Drawable::createFromImageData(BinaryData::close_svg,
                                                            BinaryData::close_svgSize)),
        close_button_(base, close_drawable_.get(), nullptr),
        bypass_drawable_(juce::Drawable::createFromImageData(BinaryData::bypass_svg,
                                                             BinaryData::bypass_svgSize)),
        bypass_button_(base, bypass_drawable_.get(), bypass_drawable_.get(),
                       tooltip_helper.getToolTipText(multilingual::kBandBypass)),
        label_laf_(base),
        freq_label_("", "FREQ"),
        gain_label_("", "GAIN"),
        q_label_("", "Q"),
        left_drawable_(juce::Drawable::createFromImageData(BinaryData::left_arrow_svg,
                                                           BinaryData::left_arrow_svgSize)),
        left_button_(base, left_drawable_.get(), nullptr),
        band_label_("", ""),
        right_drawable_(juce::Drawable::createFromImageData(BinaryData::right_arrow_svg,
                                                            BinaryData::right_arrow_svgSize)),
        right_button_(base, right_drawable_.get(), nullptr),
        ftype_box_(zlp::PFilterType::kChoices, base,
                   tooltip_helper.getToolTipText(multilingual::kBandType)),
        slope_box_(juce::StringArray{
                       "6 dB/oct  ", "12 dB/oct  ", "24 dB/oct  ", "36 dB/oct  ",
                       "48 dB/oct  ", "72 dB/oct  ", "96 dB/oct  "
                   }, base,
                   tooltip_helper.getToolTipText(multilingual::kBandSlope)),
        stereo_box_(zlp::PLRMode::kChoices, base,
                    tooltip_helper.getToolTipText(multilingual::kBandStereoMode)),
        q_slider_("", base,
                  tooltip_helper.getToolTipText(multilingual::kBandQ), 1.25f),
        dynamic_drawable_(juce::Drawable::createFromImageData(BinaryData::dynamic_svg,
                                                              BinaryData::dynamic_svgSize)),
        dynamic_button_(base, dynamic_drawable_.get(), dynamic_drawable_.get(),
                        tooltip_helper.getToolTipText(multilingual::kBandDynamic)) {
        control_background_.setBufferedToImage(true);
        addAndMakeVisible(control_background_);

        close_button_.setBufferedToImage(true);
        addAndMakeVisible(close_button_);
        close_button_.getButton().onClick = [this]() {
            if (const auto c_band = base_.getSelectedBand(); c_band < zlp::kBandNum) {
                updateValue(p_ref_.parameters_.getParameter(zlp::PFilterStatus::kID + std::to_string(c_band)),
                            0.f);
                base_.setSelectedBand(band_helper::findClosestBand(p_ref_, c_band));
            }
        };

        bypass_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        bypass_button_.setBufferedToImage(true);
        addAndMakeVisible(bypass_button_);
        bypass_button_.getButton().onClick = [this]() {
            if (const auto c_band = base_.getSelectedBand(); c_band < zlp::kBandNum) {
                updateValue(p_ref_.parameters_.getParameter(zlp::PFilterStatus::kID + std::to_string(c_band)),
                            bypass_button_.getToggleState() ? 1.f : .5f);
            }
        };

        label_laf_.setFontScale(1.5f);
        for (auto& l : {&freq_label_, &gain_label_, &q_label_}) {
            l->setLookAndFeel(&label_laf_);
            l->setJustificationType(juce::Justification::centred);
            l->setInterceptsMouseClicks(false, false);
            l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        left_button_.setBufferedToImage(true);
        addAndMakeVisible(left_button_);
        left_button_.getButton().onClick = [this]() {
            const auto c_band = base_.getSelectedBand();
            const auto next_band = band_helper::findClosestBand<false>(p_ref_, c_band);
            if (next_band < zlp::kBandNum) {
                base_.setSelectedBand(next_band);
            } else {
                base_.setSelectedBand((c_band + zlp::kBandNum - 1) % zlp::kBandNum);
            }
        };

        band_label_.setLookAndFeel(&label_laf_);
        band_label_.setJustificationType(juce::Justification::centred);
        band_label_.setInterceptsMouseClicks(false, false);
        band_label_.setBufferedToImage(true);
        addAndMakeVisible(band_label_);

        right_button_.setBufferedToImage(true);
        addAndMakeVisible(right_button_);
        right_button_.getButton().onClick = [this]() {
            const auto c_band = base_.getSelectedBand();
            const auto next_band = band_helper::findClosestBand<true>(p_ref_, c_band);
            if (next_band < zlp::kBandNum) {
                base_.setSelectedBand(next_band);
            } else {
                base_.setSelectedBand((c_band + 1) % zlp::kBandNum);
            }
        };

        const auto popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::upwards);
        slope_box_.getLAF().setItemJustification(juce::Justification::centredRight);
        for (auto& box : {&ftype_box_, &slope_box_, &stereo_box_}) {
            box->getLAF().setOption(popup_option);
            box->setBufferedToImage(true);
            addAndMakeVisible(box);
        }

        q_slider_.setBufferedToImage(true);
        addAndMakeVisible(q_slider_);

        dynamic_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        dynamic_button_.setBufferedToImage(true);
        addAndMakeVisible(dynamic_button_);
        dynamic_button_.getButton().onClick = [this]() {
            if (const auto c_band = base_.getSelectedBand(); c_band < zlp::kBandNum) {
                band_helper::turnOnOffDynamic(p_ref_, c_band, dynamic_button_.getToggleState());
            }
        };

        setInterceptsMouseClicks(false, true);
    }

    SubLeftControlPanel::~SubLeftControlPanel() = default;

    int SubLeftControlPanel::getIdealWidth() const {
        const auto font_size = base_.getFontSize();
        const auto slider_width = getSliderWidth(font_size);
        const auto button_height = getButtonSize(font_size);
        const auto padding = getPaddingSize(font_size);

        return 6 * padding + 4 * slider_width + button_height;
    }

    void SubLeftControlPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto slider_width = getSliderWidth(font_size);
        const auto button_height = getButtonSize(font_size);
        const auto box_height = getBoxHeight(font_size);
        const auto padding = getPaddingSize(font_size);

        auto bound = getLocalBounds();
        control_background_.setBounds(bound);
        bound.reduce(padding, padding);

        auto top_bound = bound.removeFromTop(button_height);
        close_button_.setBounds(top_bound.removeFromRight(button_height));
        top_bound.removeFromRight(padding);
        q_label_.setBounds(top_bound.removeFromRight(slider_width));
        top_bound.removeFromRight(padding);
        gain_label_.setBounds(top_bound.removeFromRight(slider_width));
        top_bound.removeFromRight(padding);
        freq_label_.setBounds(top_bound.removeFromRight(slider_width));
        top_bound.removeFromRight(padding);
        bypass_button_.setBounds(top_bound.removeFromLeft(button_height));

        const auto band_label_width = static_cast<int>(std::round(base_.getFontSize() * 1.5f));
        const auto arrow_width = (top_bound.getWidth() - padding - band_label_width) / 2;
        right_button_.setBounds(top_bound.removeFromRight(arrow_width));
        band_label_.setBounds(top_bound.removeFromRight(band_label_width));
        left_button_.setBounds(top_bound.removeFromRight(arrow_width));

        dynamic_button_.setBounds(bound.removeFromRight(button_height));
        bound.removeFromRight(padding);
        q_slider_.setBounds(bound.removeFromRight(slider_width));
        bound.removeFromRight(2 * slider_width + 3 * padding);

        const auto h_padding = (bound.getHeight() - 3 * box_height) / 3;
        stereo_box_.setBounds(bound.removeFromBottom(box_height));
        bound.removeFromBottom(h_padding);
        slope_box_.setBounds(bound.removeFromBottom(box_height));
        bound.removeFromBottom(h_padding);
        ftype_box_.setBounds(bound.removeFromBottom(box_height));
    }

    void SubLeftControlPanel::repaintCallBackSlow() {
        updater_.updateComponents();
        const auto filter_on = filter_status_ptr_->load(std::memory_order::relaxed) > 1.5f;
        if (filter_on != bypass_button_.getToggleState()) {
            bypass_button_.getButton().setToggleState(filter_on, juce::dontSendNotification);
        }
    }

    void SubLeftControlPanel::updateBand() {
        if (base_.getSelectedBand() < zlp::kBandNum) {
            const auto band_s = std::to_string(base_.getSelectedBand());
            ftype_attachment_ = std::make_unique<zlgui::attachment::ComboBoxAttachment<true>>(
                ftype_box_.getBox(), p_ref_.parameters_, zlp::PFilterType::kID + band_s, updater_);
            slope_attachment_ = std::make_unique<zlgui::attachment::ComboBoxAttachment<true>>(
                slope_box_.getBox(), p_ref_.parameters_, zlp::POrder::kID + band_s, updater_);
            stereo_attachment_ = std::make_unique<zlgui::attachment::ComboBoxAttachment<true>>(
                stereo_box_.getBox(), p_ref_.parameters_, zlp::PLRMode::kID + band_s, updater_);
            q_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                q_slider_.getSlider1(), p_ref_.parameters_, zlp::PQ::kID + band_s, updater_);
            dynamic_attachment_ = std::make_unique<zlgui::attachment::ButtonAttachment<true>>(
                dynamic_button_.getButton(), p_ref_.parameters_, zlp::PDynamicON::kID + band_s, updater_,
                juce::dontSendNotification);
            band_label_.setText(band_s, juce::dontSendNotification);
            filter_status_ptr_ = p_ref_.parameters_.getRawParameterValue(zlp::PFilterStatus::kID + band_s);
        } else {
            ftype_attachment_.reset();
            slope_attachment_.reset();
            stereo_attachment_.reset();
            q_attachment_.reset();
            dynamic_attachment_.reset();
            band_label_.setText("", juce::dontSendNotification);
            filter_status_ptr_ = nullptr;
        }
    }

    void SubLeftControlPanel::enableSlope6(const bool f) {
        if (!f && slope_box_.getBox().getSelectedId() == 1) {
            slope_box_.getBox().setSelectedId(2, juce::sendNotificationSync);
        }
        slope_box_.getBox().setItemEnabled(1, f);
    }

    void SubLeftControlPanel::enableGain(const bool f) {
        if (!f && dynamic_button_.getToggleState()) {
            dynamic_button_.getButton().setToggleState(false, juce::sendNotificationSync);
        }
        gain_label_.setAlpha(f ? 1.f : .5f);
    }
}
