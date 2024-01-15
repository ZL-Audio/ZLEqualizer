// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "right_control_panel.hpp"

namespace zlPanel {
    RightControlPanel::RightControlPanel(juce::AudioProcessorValueTreeState &parameters,
                                         juce::AudioProcessorValueTreeState &parametersNA,
                                         zlInterface::UIBase &base)
        : uiBase(base),
          parametersRef(parameters),
          parametersNARef(parametersNA),
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
        juce::ignoreUnused(parametersNA, parametersNARef);
        dynBypassC.setDrawable(bypassDrawable.get());
        dynSoloC.setDrawable(soloDrawable.get());
        dynRelativeC.setDrawable(relativeDrawable.get());
        sideChainC.setDrawable(sideDrawable.get());
        for (auto &c: {&dynBypassC, &dynSoloC, &dynRelativeC, &sideChainC}) {
            addAndMakeVisible(c);
        }
        for (auto &c: {&sideFreqC, &sideQC}) {
            addAndMakeVisible(c);
        }
        for (auto &c: {&thresC, &kneeC, &attackC, &releaseC}) {
            addAndMakeVisible(c);
        }
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
            Track(Fr(3)),
            Track(Fr(3)), Track(Fr(6)),
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
            const auto f = static_cast<bool>(newValue);
            dynBypassC.setEditable(f);
            dynSoloC.setEditable(f);
            dynRelativeC.setEditable(f);
            // sideChainC.setEditable(f);
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
        // sideChainC.repaint();
        repaint();
    }
}
