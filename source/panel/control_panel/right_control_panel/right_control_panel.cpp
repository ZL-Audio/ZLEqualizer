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

namespace zlPanel {
    RightControlPanel::RightControlPanel(PluginProcessor &p,
                                         zlInterface::UIBase &base)
        : uiBase(base),
          parametersRef(p.parameters),
          parametersNARef(p.parametersNA),
          dynBypassC("B", base, zlInterface::multilingual::labels::bandDynamicBypass),
          dynSoloC("S", base, zlInterface::multilingual::labels::bandDynamicSolo),
          dynRelativeC("R", base, zlInterface::multilingual::labels::bandDynamicRelative),
          swapC("S", base, zlInterface::multilingual::labels::bandSideSwap),
          sideFreqC("FREQ", base, zlInterface::multilingual::labels::bandDynamicSideFreq),
          sideQC("Q", base, zlInterface::multilingual::labels::bandDynamicSideQ),
          thresC("Threshold", base, zlInterface::multilingual::labels::bandDynamicThreshold),
          kneeC("Knee", base, zlInterface::multilingual::labels::bandDynamicKnee),
          attackC("Attack", base, zlInterface::multilingual::labels::bandDynamicAttack),
          releaseC("Release", base, zlInterface::multilingual::labels::bandDynamicRelease),
          bypassDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg, BinaryData::fadpowerswitch_svgSize)),
          soloDrawable(juce::Drawable::createFromImageData(BinaryData::fadsolo_svg, BinaryData::fadsolo_svgSize)),
          relativeDrawable(juce::Drawable::createFromImageData(BinaryData::relative_svg, BinaryData::relative_svgSize)),
          swapDrawable(juce::Drawable::createFromImageData(BinaryData::swap_svg, BinaryData::swap_svgSize)) {
        juce::ignoreUnused(parametersNARef);
        dynBypassC.setDrawable(bypassDrawable.get());
        dynBypassC.getLAF().setReverse(true);
        dynBypassC.getButton().onClick = [this]() {
            const auto isByPassed = static_cast<float>(dynBypassC.getButton().getToggleState());
            const auto currentBand = bandIdx.load();
            const auto isCurrentBandSelected = uiBase.getIsBandSelected(currentBand);
            for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
                if (idx == currentBand || (isCurrentBandSelected && uiBase.getIsBandSelected(idx))) {
                    const auto activeID = zlState::appendSuffix(zlDSP::dynamicBypass::ID, idx);
                    const auto para = parametersRef.getParameter(activeID);
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

    RightControlPanel::~RightControlPanel() {
    }

    void RightControlPanel::paint(juce::Graphics &g) {
        const auto bound = getLocalBounds().toFloat();
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
    }

    void RightControlPanel::resized() {
        // update padding
        {
            for (auto &s: {&sideFreqC, &sideQC}) {
                s->setPadding(std::round(uiBase.getFontSize() * 0.5f), 0.f);
            }

            for (auto &s: {&thresC, &kneeC, &attackC, &releaseC}) {
                s->setPadding(std::round(uiBase.getFontSize() * 0.5f),
                              std::round(uiBase.getFontSize() * 0.6f));
            }
        }
        // update bounds
        auto bound = getLocalBounds(); {
            const auto pad = static_cast<int>(uiBase.getFontSize() * .5f);
            bound = bound.withSizeKeepingCentre(bound.getWidth() - 2 * pad, bound.getHeight() - 2 * pad);
        }
        const auto buttonWidth = static_cast<int>(uiBase.getFontSize() * buttonWidthP);
        const auto buttonHeight = std::min(static_cast<int>(uiBase.getFontSize() * buttonHeightP),
                                           bound.getHeight() / 2);
        const auto sliderWidth = static_cast<int>(std::round(uiBase.getFontSize() * rSliderWidthP)); {
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
                   zlDSP::dynamicBypass::ID + suffix, zlDSP::sideSolo::ID + suffix,
                   zlDSP::dynamicRelative::ID + suffix, zlDSP::sideSwap::ID + suffix
               },
               parametersRef, buttonAttachments);
        attach({&thresC.getSlider(), &attackC.getSlider(), &kneeC.getSlider(), &releaseC.getSlider()},
               {
                   zlDSP::threshold::ID + suffix, zlDSP::attack::ID + suffix,
                   zlDSP::kneeW::ID + suffix, zlDSP::release::ID + suffix
               },
               parametersRef, sliderAttachments);
        attach({&sideFreqC.getSlider1(), &sideQC.getSlider1()},
               {zlDSP::sideFreq::ID + suffix, zlDSP::sideQ::ID + suffix},
               parametersRef, sliderAttachments);
    }

    void RightControlPanel::updateMouseDragSensitivity() {
        const auto style = uiBase.getRotaryStyle();
        const auto sensitivity = juce::roundToInt(uiBase.getRotaryDragSensitivity() * uiBase.getFontSize());
        for (auto &c: {&sideFreqC, &sideQC}) {
            c->getSlider1().setSliderStyle(style);
            c->getSlider2().setSliderStyle(style);
            c->setMouseDragSensitivity(sensitivity);
        }
    }
}
