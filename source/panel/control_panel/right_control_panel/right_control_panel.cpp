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
    RightControlPanel::RightControlPanel(PluginProcessor &p,
                                         zlgui::UIBase &base)
        : ui_base_(base),
          parameters_ref_(p.parameters_),
          parameters_NA_ref_(p.parameters_NA_),
          background_(ui_base_),
          dyn_bypass_c_("B", base, zlgui::multilingual::Labels::kBandDynamicBypass),
          dyn_solo_c_("S", base, zlgui::multilingual::Labels::kBandDynamicSolo),
          dyn_relative_c_("R", base, zlgui::multilingual::Labels::kBandDynamicRelative),
          swap_c_("S", base, zlgui::multilingual::Labels::kBandSideSwap),
          side_freq_c_("FREQ", base, zlgui::multilingual::Labels::kBandDynamicSideFreq),
          side_q_c_("Q", base, zlgui::multilingual::Labels::kBandDynamicSideQ),
          threshold_c_("Threshold", base, zlgui::multilingual::Labels::kBandDynamicThreshold),
          knee_c_("Knee", base, zlgui::multilingual::Labels::kBandDynamicKnee),
          attack_c_("Attack", base, zlgui::multilingual::Labels::kBandDynamicAttack),
          release_c_("Release", base, zlgui::multilingual::Labels::kBandDynamicRelease),
          bypass_drawable_(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg, BinaryData::fadpowerswitch_svgSize)),
          solo_drawable_(juce::Drawable::createFromImageData(BinaryData::fadsolo_svg, BinaryData::fadsolo_svgSize)),
          relative_drawable_(juce::Drawable::createFromImageData(BinaryData::relative_svg, BinaryData::relative_svgSize)),
          swap_drawable_(juce::Drawable::createFromImageData(BinaryData::swap_svg, BinaryData::swap_svgSize)) {
        juce::ignoreUnused(parameters_NA_ref_);
        addAndMakeVisible(background_);

        dyn_bypass_c_.setDrawable(bypass_drawable_.get());
        dyn_bypass_c_.getLAF().setReverse(true);
        dyn_bypass_c_.getButton().onClick = [this]() {
            const auto isByPassed = static_cast<float>(dyn_bypass_c_.getButton().getToggleState());
            const auto currentBand = band_idx_.load();
            const auto isCurrentBandSelected = ui_base_.getIsBandSelected(currentBand);
            for (size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
                if (idx == currentBand || (isCurrentBandSelected && ui_base_.getIsBandSelected(idx))) {
                    const auto activeID = zlstate::appendSuffix(zlp::dynamicBypass::ID, idx);
                    const auto para = parameters_ref_.getParameter(activeID);
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(isByPassed);
                    para->endChangeGesture();
                }
            }
        };
        dyn_solo_c_.setDrawable(solo_drawable_.get());
        dyn_relative_c_.setDrawable(relative_drawable_.get());
        swap_c_.setDrawable(swap_drawable_.get());
        dyn_relative_c_.getLAF().setScale(1.25f);
        swap_c_.getLAF().setScale(1.25f);
        for (auto &c: {&dyn_bypass_c_, &dyn_solo_c_, &dyn_relative_c_, &swap_c_}) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
        }
        for (auto &c: {&side_freq_c_, &side_q_c_}) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
        }
        for (auto &c: {&threshold_c_, &knee_c_, &attack_c_, &release_c_}) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
        }
        lookAndFeelChanged();
    }

    RightControlPanel::~RightControlPanel() = default;

    void RightControlPanel::resized() {
        // update padding
        {
            for (auto &s: {&side_freq_c_, &side_q_c_}) {
                s->setPadding(std::round(ui_base_.getFontSize() * 0.5f), 0.f);
            }

            for (auto &s: {&threshold_c_, &knee_c_, &attack_c_, &release_c_}) {
                s->setPadding(std::round(ui_base_.getFontSize() * 0.5f),
                              std::round(ui_base_.getFontSize() * 0.6f));
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
            dyn_bypass_c_.setBounds(m_bound.removeFromTop(button_height));
            dyn_solo_c_.setBounds(m_bound.removeFromBottom(button_height));
        } {
            auto m_bound = bound.removeFromLeft(button_width);
            dyn_relative_c_.setBounds(m_bound.removeFromTop(button_height));
            swap_c_.setBounds(m_bound.removeFromBottom(button_height));
        }
        side_q_c_.setBounds(bound.removeFromRight(slider_width));
        side_freq_c_.setBounds(bound.removeFromRight(slider_width));
        const auto remaining_width = bound.getWidth() / 2; {
            auto m_bound = bound.removeFromLeft(remaining_width);
            threshold_c_.setBounds(m_bound.removeFromTop(button_height));
            knee_c_.setBounds(m_bound.removeFromBottom(button_height));
        } {
            auto m_bound = bound.removeFromLeft(remaining_width);
            attack_c_.setBounds(m_bound.removeFromTop(button_height));
            release_c_.setBounds(m_bound.removeFromBottom(button_height));
        }
        // update sliders' dragging distance
        updateMouseDragSensitivity();
    }

    void RightControlPanel::lookAndFeelChanged() {
        updateMouseDragSensitivity();
    }

    void RightControlPanel::attachGroup(const size_t idx) {
        band_idx_.store(idx);
        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);

        button_attachments_.clear(true);
        slider_attachments_.clear(true);

        attach({&dyn_bypass_c_.getButton(), &dyn_solo_c_.getButton(), &dyn_relative_c_.getButton(), &swap_c_.getButton()},
               {
                   zlp::dynamicBypass::ID + suffix, zlp::sideSolo::ID + suffix,
                   zlp::dynamicRelative::ID + suffix, zlp::sideSwap::ID + suffix
               },
               parameters_ref_, button_attachments_);
        attach({&threshold_c_.getSlider(), &attack_c_.getSlider(), &knee_c_.getSlider(), &release_c_.getSlider()},
               {
                   zlp::threshold::ID + suffix, zlp::attack::ID + suffix,
                   zlp::kneeW::ID + suffix, zlp::release::ID + suffix
               },
               parameters_ref_, slider_attachments_);
        attach({&side_freq_c_.getSlider1(), &side_q_c_.getSlider1()},
               {zlp::sideFreq::ID + suffix, zlp::sideQ::ID + suffix},
               parameters_ref_, slider_attachments_);
    }

    void RightControlPanel::updateMouseDragSensitivity() {
        const auto style = ui_base_.getRotaryStyle();
        const auto sensitivity = juce::roundToInt(ui_base_.getRotaryDragSensitivity() * ui_base_.getFontSize());
        for (auto &c: {&side_freq_c_, &side_q_c_}) {
            c->getSlider1().setSliderStyle(style);
            c->getSlider2().setSliderStyle(style);
            c->setMouseDragSensitivity(sensitivity);
        }
    }
}
