// Copyright (C) 2024 - zsliu98
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
          dynBypassC("B", base),
          dynSoloC("S", base),
          dynRelativeC("R", base),
          sideChainC("S", base),
          sideFreqC("FREQ", base),
          sideQC("Q", base),
          thresC("Threshold", base),
          kneeC("Knee", base),
          attackC("Attack", base),
          releaseC("Release", base),
          bypassDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg, BinaryData::fadpowerswitch_svgSize)),
          soloDrawable(juce::Drawable::createFromImageData(BinaryData::fadsolo_svg, BinaryData::fadsolo_svgSize)),
          relativeDrawable(juce::Drawable::createFromImageData(BinaryData::righttobracketsolid_svg,
                                                               BinaryData::righttobracketsolid_svgSize)),
          sideDrawable(juce::Drawable::createFromImageData(BinaryData::fadside_svg, BinaryData::fadside_svgSize)) {
        juce::ignoreUnused(parametersNARef);
        dynBypassC.setDrawable(bypassDrawable.get());
        dynBypassC.getLAF().setReverse(true);
        dynBypassC.getButton().onClick = [this]() {
            const auto isByPassed = static_cast<float>(dynBypassC.getButton().getToggleState());
            const auto currentBand = bandIdx.load();
            const auto isCurrentBandSelected = uiBase.getIsBandSelected(currentBand);
            for(size_t idx = 0; idx < zlState::bandNUM; ++idx) {
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
        sideChainC.setDrawable(sideDrawable.get());
        sideChainC.getButton().onClick = [this]() {
            const auto isSideOn = static_cast<int>(sideChainC.getButton().getToggleState());
            const auto para = parametersNARef.getParameter(zlState::fftSideON::ID);
            para->beginChangeGesture();
            para->setValueNotifyingHost(zlState::fftSideON::convertTo01(isSideOn));
            para->endChangeGesture();
        };
        for (auto &c: {&dynBypassC, &dynSoloC, &dynRelativeC, &sideChainC}) {
            addAndMakeVisible(c);
        }
        for (auto &c: {&sideFreqC, &sideQC}) {
            addAndMakeVisible(c);
        }
        for (auto &c: {&thresC, &kneeC, &attackC, &releaseC}) {
            addAndMakeVisible(c);
        }
        lookAndFeelChanged();
    }

    RightControlPanel::~RightControlPanel() {
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            const std::string suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            parametersRef.removeParameterListener(zlDSP::dynamicON::ID + suffix, this);
        }
    }

    void RightControlPanel::paint(juce::Graphics &g) {
        const auto bound = getLocalBounds().toFloat();
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
    }

    void RightControlPanel::resized() {
        juce::Grid grid;
        using Track = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;

        grid.templateRows = {Track(Fr(1)), Track(Fr(1))};
        grid.templateColumns = {
            Track(Fr(3)), Track(Fr(3)), Track(Fr(6)),
            Track(Fr(6)), Track(Fr(6)), Track(Fr(6))
        };
        grid.items = {
            juce::GridItem(dynBypassC).withArea(1, 1),
            juce::GridItem(dynRelativeC).withArea(1, 2),
            juce::GridItem(thresC).withArea(1, 3),
            juce::GridItem(attackC).withArea(1, 4),
            juce::GridItem(sideFreqC).withArea(1, 5, 3, 6),
            juce::GridItem(sideQC).withArea(1, 6, 3, 7),
            juce::GridItem(dynSoloC).withArea(2, 1),
            juce::GridItem(sideChainC).withArea(2, 2),
            juce::GridItem(kneeC).withArea(2, 3),
            juce::GridItem(releaseC).withArea(2, 4),
        };

        for (auto &s: {&sideFreqC, &sideQC}) {
            s->setPadding(uiBase.getFontSize() * 0.5f, 0.f);
        }

        for (auto &s: {&thresC, &kneeC, &attackC, &releaseC}) {
            s->setPadding(uiBase.getFontSize() * 0.5f, 0.f);
        }

        auto bound = getLocalBounds().toFloat();
        bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {});
        grid.performLayout(bound.toNearestInt());
        updateMouseDragSensitivity();
    }

    void RightControlPanel::lookAndFeelChanged() {
        updateMouseDragSensitivity();
    }

    void RightControlPanel::attachGroup(size_t idx) {
        const std::string oldSuffix = bandIdx.load() < 10
                                          ? "0" + std::to_string(bandIdx.load())
                                          : std::to_string(bandIdx.load());
        parametersRef.removeParameterListener(zlDSP::dynamicON::ID + oldSuffix, this);

        bandIdx.store(idx);
        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        parametersRef.addParameterListener(zlDSP::dynamicON::ID + suffix, this);

        buttonAttachments.clear(true);
        sliderAttachments.clear(true);

        attach({&dynBypassC.getButton(), &dynSoloC.getButton(), &dynRelativeC.getButton(), &sideChainC.getButton()},
               {
                   zlDSP::dynamicBypass::ID + suffix, zlDSP::sideSolo::ID + suffix,
                   zlDSP::dynamicRelative::ID + suffix, zlDSP::sideChain::ID
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
        parameterChanged(zlDSP::dynamicON::ID + suffix,
                         parametersRef.getRawParameterValue(zlDSP::dynamicON::ID + suffix)->load());
    }

    void RightControlPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        const auto id = parameterID.dropLastCharacters(2);
        const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        if (id == zlDSP::dynamicON::ID) {
            const auto f = newValue > .5f;
            dynBypassC.setEditable(f);
            dynSoloC.setEditable(f);
            dynRelativeC.setEditable(f);
            thresC.setEditable(f);
            attackC.setEditable(f);
            kneeC.setEditable(f);
            releaseC.setEditable(f);
            sideFreqC.setEditable(f);
            sideQC.setEditable(f);
            if (idx == bandIdx.load()) {
                triggerAsyncUpdate();
            }
        }
    }

    void RightControlPanel::handleAsyncUpdate() {
        dynBypassC.repaint();
        dynSoloC.repaint();
        dynRelativeC.repaint();
        repaint();
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
