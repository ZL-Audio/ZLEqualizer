// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "float_pop_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    FloatPopPanel::FloatPopPanel(PluginProcessor& p, zlgui::UIBase& base,
                                 const multilingual::TooltipHelper& tooltip_helper) :
        p_ref_(p), base_(base), updater_(),
        control_background_(base, .25f),
        bypass_drawable_(juce::Drawable::createFromImageData(BinaryData::bypass_svg,
                                                             BinaryData::bypass_svgSize)),
        bypass_button_(base, bypass_drawable_.get(), bypass_drawable_.get(),
                       tooltip_helper.getToolTipText(multilingual::kBandBypass)),
        solo_drawable_(juce::Drawable::createFromImageData(BinaryData::solo_svg, BinaryData::solo_svgSize)),
        solo_button_(base, solo_drawable_.get(), solo_drawable_.get(),
                     tooltip_helper.getToolTipText(multilingual::kBandSolo)),
        close_drawable_(juce::Drawable::createFromImageData(BinaryData::close_svg,
                                                            BinaryData::close_svgSize)),
        close_button_(base, close_drawable_.get(), nullptr),
        ftype_box_([]() -> std::vector<std::unique_ptr<juce::Drawable>> {
            std::vector<std::unique_ptr<juce::Drawable>> icons;
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::peak_svg, BinaryData::peak_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::lowshelf_svg, BinaryData::lowshelf_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::lowpass_svg, BinaryData::lowpass_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::highshelf_svg, BinaryData::highshelf_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::highpass_svg, BinaryData::highpass_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::notch_svg, BinaryData::notch_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::bandpass_svg, BinaryData::bandpass_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::tiltshelf_svg, BinaryData::tiltshelf_svgSize));
            return icons;
        }(), base, "", {}),
        lr_box_([]() -> std::vector<std::unique_ptr<juce::Drawable>> {
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
        }(), base, "", {}),
        freq_slider_("", base) {
        control_background_.setBufferedToImage(true);
        addAndMakeVisible(control_background_);

        close_button_.setBufferedToImage(true);
        addAndMakeVisible(close_button_);
        close_button_.getButton().onClick = [this]() {
            if (const auto c_band = base_.getSelectedBand(); c_band < zlp::kBandNum) {
                band_helper::turnOffBand(p_ref_, c_band, base_.getSelectedBandSet());
                const auto band1 = band_helper::findClosestBand<true>(p_ref_, c_band);
                const auto band2 = band_helper::findClosestBand<false>(p_ref_, c_band);
                if (band1 < zlp::kBandNum) {
                    base_.setSelectedBand(band1);
                } else if (band2 < zlp::kBandNum) {
                    base_.setSelectedBand(band2);
                } else {
                    base_.setSelectedBand(zlp::kBandNum);
                }
            }
        };

        solo_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        solo_button_.setBufferedToImage(true);
        addAndMakeVisible(solo_button_);
        solo_button_.getButton().onClick = [this]() {
            if (solo_button_.getButton().getToggleState()) {
                if (const auto c_band = base_.getSelectedBand(); c_band < zlp::kBandNum) {
                    base_.setSoloWholeIdx(c_band);
                }
            } else {
                base_.setSoloWholeIdx(2 * zlp::kBandNum);
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

        const auto popup_option1 = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::upwards).withMinimumNumColumns(8);
        ftype_box_.getLAF().setOption(popup_option1);
        ftype_box_.setBufferedToImage(true);
        addAndMakeVisible(ftype_box_);

        const auto popup_option2 = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::upwards).withMinimumNumColumns(5);
        lr_box_.getLAF().setOption(popup_option2);
        lr_box_.setBufferedToImage(true);
        addAndMakeVisible(lr_box_);

        freq_slider_.setFontScale(1.25f);
        freq_slider_.getSlider().setDraggingEnabled(false);
        freq_slider_.getSlider().setSliderSnapsToMousePosition(false);
        freq_slider_.getSlider().setScrollWheelEnabled(false);
        freq_slider_.permitted_characters_ = "-0123456789.kK#ABCDEFG";
        freq_slider_.value_formatter_ = freq_note::getNoteFromFrequency;
        freq_slider_.string_formatter_ = freq_note::getFrequencyFromNote;
        freq_slider_.setBufferedToImage(true);
        addAndMakeVisible(freq_slider_);

        base_.getSoloWholeIdxTree().addListener(this);
    }

    FloatPopPanel::~FloatPopPanel() {
        base_.getSoloWholeIdxTree().removeListener(this);
    }

    void FloatPopPanel::resized() {
        control_background_.setBounds(getLocalBounds());

        const auto padding = getPaddingSize(base_.getFontSize());
        const auto button_size = getButtonSize(base_.getFontSize());
        auto bound = getLocalBounds().reduced(padding);

        auto top_bound = bound.removeFromTop(button_size);
        bypass_button_.setBounds(top_bound.removeFromLeft(button_size));
        top_bound.removeFromLeft(padding);
        {
            auto t_bound = top_bound.removeFromLeft(button_size);
            t_bound = t_bound.reduced(solo_button_.getButton().getEdgeIndent() / 2);
            ftype_box_.setBounds(t_bound);
        }
        top_bound.removeFromLeft(padding);
        {
            auto t_bound = top_bound.removeFromLeft(button_size);
            t_bound = t_bound.reduced(solo_button_.getButton().getEdgeIndent() / 2);
            lr_box_.setBounds(t_bound);
        }

        auto bottom_bound = bound.removeFromBottom(button_size);
        solo_button_.setBounds(bottom_bound.removeFromLeft(button_size));
        close_button_.setBounds(bottom_bound.removeFromRight(button_size));
        freq_slider_.setBounds(bottom_bound);

        ideal_height_ = static_cast<float>(getIdealHeight());
        ideal_width_ = static_cast<float>(getIdealWidth());
    }

    void FloatPopPanel::updateBand() {
        if (base_.getSelectedBand() < zlp::kBandNum) {
            const auto band_s = std::to_string(base_.getSelectedBand());
            ftype_attachment_ = std::make_unique<zlgui::attachment::ComboBoxAttachment<true>>(
                ftype_box_.getBox(), p_ref_.parameters_, zlp::PFilterType::kID + band_s, updater_);
            lr_attachment_ = std::make_unique<zlgui::attachment::ComboBoxAttachment<true>>(
                lr_box_.getBox(), p_ref_.parameters_, zlp::PLRMode::kID + band_s, updater_);
            freq_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                freq_slider_.getSlider(), p_ref_.parameters_, zlp::PFreq::kID + band_s, updater_);
            freq_attachment_->updateComponent();
            freq_slider_.updateDisplayValue();
            filter_status_ptr_ = p_ref_.parameters_.getRawParameterValue(zlp::PFilterStatus::kID + band_s);
            repaintCallBackSlow();
        } else {
            ftype_attachment_.reset();
            lr_attachment_.reset();
            freq_attachment_.reset();
            filter_status_ptr_ = nullptr;
        }
        setVisible(base_.getSelectedBand() < zlp::kBandNum);
    }

    void FloatPopPanel::repaintCallBackSlow() {
        if (filter_status_ptr_ != nullptr) {
            updater_.updateComponents();
            const auto filter_on = filter_status_ptr_->load(std::memory_order::relaxed) > 1.5f;
            if (filter_on != bypass_button_.getToggleState()) {
                bypass_button_.getButton().setToggleState(filter_on, juce::dontSendNotification);
            }
        }
    }

    int FloatPopPanel::getIdealWidth() const {
        const auto padding = getPaddingSize(base_.getFontSize());
        const auto button_size = getButtonSize(base_.getFontSize());
        return 3 * button_size + 4 * padding;
    }

    int FloatPopPanel::getIdealHeight() const {
        const auto padding = getPaddingSize(base_.getFontSize());
        const auto button_size = getButtonSize(base_.getFontSize());
        return 2 * button_size + 3 * padding;
    }

    void FloatPopPanel::setTargetVisible(const bool is_target_visible) {
        if (is_target_visible != is_target_visible_) {
            is_target_visible_ = is_target_visible;
            updateTransformation();
        }
    }

    void FloatPopPanel::updatePosition(const juce::Point<float> position) {
        if (std::abs(position.x - position_.x) > .01f
            || std::abs(position.y - position_.y) > .01f) {
            position_ = position;
            updateTransformation();
        }
    }

    void FloatPopPanel::updateFloatingBound(const juce::Rectangle<float> bound) {
        const auto width = ideal_width_;
        const auto height = ideal_height_;
        const auto padding = base_.getFontSize() * kPaddingScale * 1.5f;

        upper_center_ = {width * .5f, -base_.getFontSize()};
        lower_center_ = {width * .5f, height + base_.getFontSize()};
        left_center_ = {-padding, ideal_height_ * .5f};
        right_center_ = {width + padding, ideal_height_ * .5f};

        x_min_ = width * .5f;
        x_mid_ = bound.getWidth() * .5f;
        x_max_ = bound.getWidth() - width * .5f;
        y_min_ = ideal_height_ * .5f;
        y_max_ = bound.getHeight() - height * .5f;

        y1_ = bound.getHeight() * .25f;
        y2_ = bound.getHeight() * .5f + .5f;
        y3_ = bound.getHeight() * .75f;

        updateTransformation();
    }

    void FloatPopPanel::updateTransformation() {
        if (is_target_visible_) {
            if (position_.x < x_mid_) {
                setTransform(juce::AffineTransform::translation(
                    position_.x - left_center_.x,
                    std::clamp(position_.y, y_min_, y_max_) - left_center_.y));
            } else {
                setTransform(juce::AffineTransform::translation(
                    position_.x - right_center_.x,
                    std::clamp(position_.y, y_min_, y_max_) - right_center_.y));
            }
        } else if (position_.y < y1_ || (position_.y > y2_ && position_.y < y3_)) {
            setTransform(juce::AffineTransform::translation(
                std::clamp(position_.x, x_min_, x_max_) - upper_center_.x,
                position_.y - upper_center_.y));
        } else {
            setTransform(juce::AffineTransform::translation(
                std::clamp(position_.x, x_min_, x_max_) - lower_center_.x,
                position_.y - lower_center_.y));
        }
    }

    void FloatPopPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) {
        const auto solo_whole_idx = base_.getSoloWholeIdx();
        solo_button_.getButton().setToggleState(solo_whole_idx < zlp::kBandNum, juce::dontSendNotification);
    }
}
