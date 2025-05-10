// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "left_control_panel.hpp"

namespace zlpanel {
    LeftControlPanel::LeftControlPanel(PluginProcessor &p,
                                       zlgui::UIBase &base)
        : processor_ref_(p), ui_base_(base),
          parameters_ref_(p.parameters_),
          parameters_NA_ref_(p.parameters_NA_),
          background_(ui_base_),
          bypass_c_("B", base, zlgui::multilingual::Labels::kBandBypass),
          solo_c_("S", base, zlgui::multilingual::Labels::kBandSolo),
          dyn_on_c_("D", base, zlgui::multilingual::Labels::kBandDynamic),
          dyn_l_c_("L", base, zlgui::multilingual::Labels::kBandDynamicAuto),
          f_type_c_("", zlp::fType::choices, base, zlgui::multilingual::Labels::kBandType),
          slope_c_("", zlp::slope::choices, base, zlgui::multilingual::Labels::kBandSlope),
          stereo_c_("", zlp::lrType::choices, base, zlgui::multilingual::Labels::kBandStereoMode),
          lr_box_(zlstate::selectedBandIdx::choices, base, zlgui::multilingual::Labels::kBandSelector),
          freq_c_("FREQ", base, zlgui::multilingual::Labels::kBandFreq),
          gain_c_("GAIN", base, zlgui::multilingual::Labels::kBandGain),
          q_c_("Q", base, zlgui::multilingual::Labels::kBandQ),
          reset_component_(p.parameters_, p.parameters_NA_, base),
          bypass_drawable_(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg, BinaryData::fadpowerswitch_svgSize)),
          solo_drawable_(juce::Drawable::createFromImageData(BinaryData::fadsolo_svg, BinaryData::fadsolo_svgSize)),
          dyn_on_drawable_(
              juce::Drawable::createFromImageData(BinaryData::fadmodsine_svg, BinaryData::fadmodsine_svgSize)),
          dyn_l_drawable_(
              juce::Drawable::createFromImageData(BinaryData::fadpreseta_svg, BinaryData::fadpreseta_svgSize)) {
        juce::ignoreUnused(parameters_NA_ref_);
        addAndMakeVisible(background_);
        bypass_c_.setDrawable(bypass_drawable_.get());
        bypass_c_.getLAF().setReverse(true);
        bypass_c_.getButton().onClick = [this]() {
            const auto isByPassed = static_cast<float>(bypass_c_.getButton().getToggleState());
            const auto currentBand = band_idx_.load();
            const auto isCurrentBandSelected = ui_base_.getIsBandSelected(currentBand);
            for (size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
                if (idx == currentBand || (isCurrentBandSelected && ui_base_.getIsBandSelected(idx))) {
                    const auto paraBypass = parameters_ref_.getParameter(zlstate::appendSuffix(zlp::bypass::ID, idx));
                    paraBypass->beginChangeGesture();
                    paraBypass->setValueNotifyingHost(isByPassed);
                    paraBypass->endChangeGesture();
                }
            }
        };

        solo_c_.setDrawable(solo_drawable_.get());
        dyn_on_c_.setDrawable(dyn_on_drawable_.get());
        dyn_on_c_.getButton().onClick = [this]() {
            const auto currentBand = band_idx_.load();
            float dynLinkValue = 0.0;
            if (dyn_on_c_.getButton().getToggleState()) {
                processor_ref_.getFiltersAttach().turnOnDynamic(currentBand);
                dynLinkValue = static_cast<float>(ui_base_.getDynLink()); {
                    auto *para = parameters_ref_.getParameter(
                        zlp::appendSuffix(zlp::bypass::ID, currentBand));
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(0.f);
                    para->endChangeGesture();
                }
            } else {
                processor_ref_.getFiltersAttach().turnOffDynamic(currentBand);
            } {
                auto *para = parameters_ref_.getParameter(
                    zlp::appendSuffix(zlp::singleDynLink::ID, currentBand));
                para->beginChangeGesture();
                para->setValueNotifyingHost(dynLinkValue);
                para->endChangeGesture();
            }
        };

        dyn_l_c_.setDrawable(dyn_l_drawable_.get());
        dyn_l_c_.getButton().onClick = [this]() {
            const auto currentBand = band_idx_.load();
            if (dyn_l_c_.getButton().getToggleState()) {
                processor_ref_.getFiltersAttach().turnOnDynamicAuto(currentBand);
            }
        };

        for (auto &c: {&bypass_c_, &solo_c_, &dyn_on_c_, &dyn_l_c_}) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
        }
        for (auto &c: {&f_type_c_, &slope_c_, &stereo_c_}) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
        }
        for (auto &c: {&freq_c_}) {
            addAndMakeVisible(c);
        }
        freq_c_.parseString = [](const juce::String s) {
            if (const auto v = parseFreqPitchString(s)) {
                return v.value();
            } else {
                return parseFreqValueString(s);
            }
        };
        freq_c_.allowedChars = juce::String("0123456789.kKABCDEFGabcdefg#");
        q_c_.setBufferedToImage(true);
        for (auto &c: {&gain_c_, &q_c_}) {
            addAndMakeVisible(c);
        }
        gain_c_.allowedChars = juce::String("-0123456789.");
        q_c_.allowedChars = juce::String("0123456789.");
        lr_box_.setBufferedToImage(true);
        addAndMakeVisible(lr_box_);
        reset_component_.setBufferedToImage(true);
        addAndMakeVisible(reset_component_);
    }

    LeftControlPanel::~LeftControlPanel() {
        for (size_t i = 0; i < zlp::kBandNUM; ++i) {
            const std::string suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            parameters_ref_.removeParameterListener(zlp::fType::ID + suffix, this);
            parameters_ref_.removeParameterListener(zlp::dynamicON::ID + suffix, this);
        }
    }

    void LeftControlPanel::resized() {
        // update padding
        {
            for (auto &s: {&freq_c_}) {
                s->setPadding(std::round(ui_base_.getFontSize() * 0.5f), 0.f);
            }
            for (auto &s: {&gain_c_, &q_c_}) {
                s->setPadding(std::round(ui_base_.getFontSize() * 0.5f), 0.f);
            }
        }
        // update bounds
        auto bound = getLocalBounds();
        background_.setBounds(bound); {
            const auto pad = static_cast<int>(ui_base_.getFontSize() * .5f);
            bound = bound.withSizeKeepingCentre(bound.getWidth() - 2 * pad, bound.getHeight() - 2 * pad);
        }
        const auto button_width = static_cast<int>(ui_base_.getFontSize() * kButtonWidthP);
        const auto button_height = std::min(static_cast<int>(ui_base_.getFontSize() * kButtonHeightP),
                                           bound.getHeight() / 2);
        const auto slider_width = static_cast<int>(std::round(ui_base_.getFontSize() * kRotarySliderWidthP)); {
            auto m_bound = bound.removeFromLeft(button_width);
            bypass_c_.setBounds(m_bound.removeFromTop(button_height));
            solo_c_.setBounds(m_bound.removeFromBottom(button_height));
        } {
            auto m_bound = bound.removeFromLeft(bound.getWidth() - (button_width * 2 + slider_width * 3));
            const auto box_height = m_bound.getHeight() / 3;
            f_type_c_.setBounds(m_bound.removeFromTop(box_height));
            slope_c_.setBounds(m_bound.removeFromTop(box_height));
            stereo_c_.setBounds(m_bound.removeFromTop(box_height));
        }
        freq_c_.setBounds(bound.removeFromLeft(slider_width));
        gain_c_.setBounds(bound.removeFromLeft(slider_width));
        q_c_.setBounds(bound.removeFromLeft(slider_width));
        const auto reset_width = juce::roundToInt(1.5f * ui_base_.getFontSize()); {
            auto m_bound = bound.removeFromBottom(button_height);
            dyn_on_c_.setBounds(m_bound.removeFromLeft(button_width));
            dyn_l_c_.setBounds(m_bound);
            m_bound = bound;
            m_bound.removeFromLeft(juce::roundToInt(0.5f * ui_base_.getFontSize()));
            m_bound.removeFromRight(juce::roundToInt(1.75f * ui_base_.getFontSize()));
            lr_box_.setBounds(m_bound);
        } {
            bound = bound.removeFromRight(reset_width);
            bound = bound.removeFromTop(reset_width);
            reset_component_.setBounds(bound);
        }
        // update sliders' dragging distance
        updateMouseDragSensitivity();
    }

    void LeftControlPanel::lookAndFeelChanged() {
        updateMouseDragSensitivity();
    }

    void LeftControlPanel::attachGroup(const size_t idx) {
        const std::string oldSuffix = band_idx_.load() < 10
                                          ? "0" + std::to_string(band_idx_.load())
                                          : std::to_string(band_idx_.load());
        parameters_ref_.removeParameterListener(zlp::fType::ID + oldSuffix, this);
        parameters_ref_.removeParameterListener(zlp::dynamicON::ID + oldSuffix, this);

        band_idx_.store(idx);
        reset_component_.attachGroup(idx);
        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        parameters_ref_.addParameterListener(zlp::fType::ID + suffix, this);
        parameters_ref_.addParameterListener(zlp::dynamicON::ID + suffix, this);

        button_attachments_.clear(true);
        box_attachments_.clear(true);
        slider_attachments_.clear(true);

        attach({&bypass_c_.getButton(), &solo_c_.getButton(), &dyn_on_c_.getButton(), &dyn_l_c_.getButton()},
               {
                   zlp::bypass::ID + suffix, zlp::solo::ID + suffix,
                   zlp::dynamicON::ID + suffix, zlp::dynamicLearn::ID + suffix
               },
               parameters_ref_, button_attachments_);
        attach({&f_type_c_.getBox(), &slope_c_.getBox(), &stereo_c_.getBox()},
               {zlp::fType::ID + suffix, zlp::slope::ID + suffix, zlp::lrType::ID + suffix},
               parameters_ref_, box_attachments_);
        attach({&lr_box_.getBox()},
               {zlstate::selectedBandIdx::ID},
               parameters_NA_ref_, box_attachments_);
        attach({&freq_c_.getSlider1(), &gain_c_.getSlider1(), &gain_c_.getSlider2(), &q_c_.getSlider1(), &q_c_.getSlider2()},
               {
                   zlp::freq::ID + suffix, zlp::gain::ID + suffix, zlp::targetGain::ID + suffix,
                   zlp::Q::ID + suffix, zlp::targetQ::ID + suffix
               },
               parameters_ref_, slider_attachments_);
        freq_c_.updateDisplay();
        gain_c_.updateDisplay();
        q_c_.updateDisplay();
        parameterChanged(zlp::fType::ID + suffix,
                         parameters_ref_.getRawParameterValue(zlp::fType::ID + suffix)->load());
        parameterChanged(zlp::dynamicON::ID + suffix,
                         parameters_ref_.getRawParameterValue(zlp::dynamicON::ID + suffix)->load());
    }

    void LeftControlPanel::parameterChanged(const juce::String &parameter_id, float new_value) {
        const auto idx = static_cast<size_t>(parameter_id.getTrailingIntValue());
        if (parameter_id.startsWith(zlp::fType::ID)) {
            switch (static_cast<zldsp::filter::FilterType>(new_value)) {
                case zldsp::filter::FilterType::kPeak:
                case zldsp::filter::FilterType::kLowShelf:
                case zldsp::filter::FilterType::kHighShelf:
                case zldsp::filter::FilterType::kBandShelf:
                case zldsp::filter::FilterType::kTiltShelf: {
                    gain_c_editable_.store(true);
                    break;
                }
                case zldsp::filter::FilterType::kLowPass:
                case zldsp::filter::FilterType::kHighPass:
                case zldsp::filter::FilterType::kBandPass:
                case zldsp::filter::FilterType::kNotch: {
                    gain_c_editable_.store(false);
                    break;
                }
            }
            switch (static_cast<zldsp::filter::FilterType>(new_value)) {
                case zldsp::filter::FilterType::kLowPass:
                case zldsp::filter::FilterType::kHighPass:
                case zldsp::filter::FilterType::kLowShelf:
                case zldsp::filter::FilterType::kHighShelf:
                case zldsp::filter::FilterType::kBandShelf:
                case zldsp::filter::FilterType::kTiltShelf: {
                    slop_c_enable_.store(true);
                    break;
                }
                case zldsp::filter::FilterType::kPeak:
                case zldsp::filter::FilterType::kBandPass:
                case zldsp::filter::FilterType::kNotch: {
                    slop_c_enable_.store(false);
                    break;
                }
            }
            if (idx == band_idx_.load()) {
                triggerAsyncUpdate();
            }
        } else if (parameter_id.startsWith(zlp::dynamicON::ID)) {
            const auto f = new_value > .5f; {
                gain_s2_editable_.store(gain_c_editable_.load() && f);
                q_s2_editable_.store(f);
            }
            if (idx == band_idx_.load()) {
                triggerAsyncUpdate();
            }
        }
    }

    void LeftControlPanel::handleAsyncUpdate() {
        gain_c_.setEditable(gain_c_editable_.load());
        slope_c_.getBox().setItemEnabled(1, slop_c_enable_.load());
        gain_c_.setShowSlider2(gain_s2_editable_.load());
        q_c_.setShowSlider2(q_s2_editable_.load());
        repaint();
    }

    void LeftControlPanel::updateMouseDragSensitivity() {
        const auto style = ui_base_.getRotaryStyle();
        const auto sensitivity = juce::roundToInt(ui_base_.getRotaryDragSensitivity() * ui_base_.getFontSize());
        for (auto &c: {&freq_c_}) {
            c->getSlider1().setSliderStyle(style);
            c->getSlider2().setSliderStyle(style);
            c->setMouseDragSensitivity(sensitivity);
        }
        for (auto &c: {&gain_c_, &q_c_}) {
            c->getSlider1().setSliderStyle(style);
            c->getSlider2().setSliderStyle(style);
            c->setMouseDragSensitivity(sensitivity);
        }
    }
}
