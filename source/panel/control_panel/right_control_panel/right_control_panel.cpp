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
          parameters_ref_(p.parameters),
          parameters_NA_ref_(p.parameters_NA),
          background(ui_base_),
          dynBypassC("B", base, zlgui::multilingual::Labels::kBandDynamicBypass),
          dynSoloC("S", base, zlgui::multilingual::Labels::kBandDynamicSolo),
          dynRelativeC("R", base, zlgui::multilingual::Labels::kBandDynamicRelative),
          swapC("S", base, zlgui::multilingual::Labels::kBandSideSwap),
          sideFreqC("FREQ", base, zlgui::multilingual::Labels::kBandDynamicSideFreq),
          sideQC("Q", base, zlgui::multilingual::Labels::kBandDynamicSideQ),
          thresC("Threshold", base, zlgui::multilingual::Labels::kBandDynamicThreshold),
          kneeC("Knee", base, zlgui::multilingual::Labels::kBandDynamicKnee),
          attackC("Attack", base, zlgui::multilingual::Labels::kBandDynamicAttack),
          releaseC("Release", base, zlgui::multilingual::Labels::kBandDynamicRelease),
          bypassDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg, BinaryData::fadpowerswitch_svgSize)),
          soloDrawable(juce::Drawable::createFromImageData(BinaryData::fadsolo_svg, BinaryData::fadsolo_svgSize)),
          relativeDrawable(juce::Drawable::createFromImageData(BinaryData::relative_svg, BinaryData::relative_svgSize)),
          swapDrawable(juce::Drawable::createFromImageData(BinaryData::swap_svg, BinaryData::swap_svgSize)) {
        juce::ignoreUnused(parameters_NA_ref_);
        addAndMakeVisible(background);

        dynBypassC.setDrawable(bypassDrawable.get());
        dynBypassC.getLAF().setReverse(true);
        dynBypassC.getButton().onClick = [this]() {
            const auto isByPassed = static_cast<float>(dynBypassC.getButton().getToggleState());
            const auto currentBand = bandIdx.load();
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
        dynSoloC.setDrawable(soloDrawable.get());
        dynRelativeC.setDrawable(relativeDrawable.get());
        swapC.setDrawable(swapDrawable.get());
        dynRelativeC.getLAF().setScale(1.25f);
        swapC.getLAF().setScale(1.25f);
        for (auto &c: {&dynBypassC, &dynSoloC, &dynRelativeC, &swapC}) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
        }
        for (auto &c: {&sideFreqC, &sideQC}) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
        }
        for (auto &c: {&thresC, &kneeC, &attackC, &releaseC}) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
        }
        lookAndFeelChanged();
    }

    RightControlPanel::~RightControlPanel() = default;

    void RightControlPanel::resized() {
        // update padding
        {
            for (auto &s: {&sideFreqC, &sideQC}) {
                s->setPadding(std::round(ui_base_.getFontSize() * 0.5f), 0.f);
            }

            for (auto &s: {&thresC, &kneeC, &attackC, &releaseC}) {
                s->setPadding(std::round(ui_base_.getFontSize() * 0.5f),
                              std::round(ui_base_.getFontSize() * 0.6f));
            }
        }
        // update bounds
        auto bound = getLocalBounds();
        background.setBounds(bound); {
            const auto pad = static_cast<int>(ui_base_.getFontSize() * .5f);
            bound = bound.withSizeKeepingCentre(bound.getWidth() - 2 * pad, bound.getHeight() - 2 * pad);
        }
        const auto buttonWidth = static_cast<int>(ui_base_.getFontSize() * buttonWidthP);
        const auto buttonHeight = std::min(static_cast<int>(ui_base_.getFontSize() * buttonHeightP),
                                           bound.getHeight() / 2);
        const auto sliderWidth = static_cast<int>(std::round(ui_base_.getFontSize() * rSliderWidthP)); {
            auto mBound = bound.removeFromLeft(buttonWidth);
            dynBypassC.setBounds(mBound.removeFromTop(buttonHeight));
            dynSoloC.setBounds(mBound.removeFromBottom(buttonHeight));
        } {
            auto mBound = bound.removeFromLeft(buttonWidth);
            dynRelativeC.setBounds(mBound.removeFromTop(buttonHeight));
            swapC.setBounds(mBound.removeFromBottom(buttonHeight));
        }
        sideQC.setBounds(bound.removeFromRight(sliderWidth));
        sideFreqC.setBounds(bound.removeFromRight(sliderWidth));
        const auto remainingWidth = bound.getWidth() / 2; {
            auto mBound = bound.removeFromLeft(remainingWidth);
            thresC.setBounds(mBound.removeFromTop(buttonHeight));
            kneeC.setBounds(mBound.removeFromBottom(buttonHeight));
        } {
            auto mBound = bound.removeFromLeft(remainingWidth);
            attackC.setBounds(mBound.removeFromTop(buttonHeight));
            releaseC.setBounds(mBound.removeFromBottom(buttonHeight));
        }
        // update sliders' dragging distance
        updateMouseDragSensitivity();
    }

    void RightControlPanel::lookAndFeelChanged() {
        updateMouseDragSensitivity();
    }

    void RightControlPanel::attachGroup(const size_t idx) {
        bandIdx.store(idx);
        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);

        buttonAttachments.clear(true);
        sliderAttachments.clear(true);

        attach({&dynBypassC.getButton(), &dynSoloC.getButton(), &dynRelativeC.getButton(), &swapC.getButton()},
               {
                   zlp::dynamicBypass::ID + suffix, zlp::sideSolo::ID + suffix,
                   zlp::dynamicRelative::ID + suffix, zlp::sideSwap::ID + suffix
               },
               parameters_ref_, buttonAttachments);
        attach({&thresC.getSlider(), &attackC.getSlider(), &kneeC.getSlider(), &releaseC.getSlider()},
               {
                   zlp::threshold::ID + suffix, zlp::attack::ID + suffix,
                   zlp::kneeW::ID + suffix, zlp::release::ID + suffix
               },
               parameters_ref_, sliderAttachments);
        attach({&sideFreqC.getSlider1(), &sideQC.getSlider1()},
               {zlp::sideFreq::ID + suffix, zlp::sideQ::ID + suffix},
               parameters_ref_, sliderAttachments);
    }

    void RightControlPanel::updateMouseDragSensitivity() {
        const auto style = ui_base_.getRotaryStyle();
        const auto sensitivity = juce::roundToInt(ui_base_.getRotaryDragSensitivity() * ui_base_.getFontSize());
        for (auto &c: {&sideFreqC, &sideQC}) {
            c->getSlider1().setSliderStyle(style);
            c->getSlider2().setSliderStyle(style);
            c->setMouseDragSensitivity(sensitivity);
        }
    }
}
