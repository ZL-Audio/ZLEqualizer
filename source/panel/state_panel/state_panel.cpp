// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "state_panel.hpp"

namespace zlPanel {
    StatePanel::StatePanel(PluginProcessor &p,
                           zlInterface::UIBase &base,
                           UISettingPanel &uiSettingPanel)
        : uiBase(base), parametersNARef(p.parametersNA),
          logoPanel(p, base, uiSettingPanel),
          fftSettingPanel(p, base),
          compSettingPanel(p, base),
          outputSettingPanel(p, base),
          conflictSettingPanel(p, base),
          generalSettingPanel(p, base),
          matchSettingPanel(base),
          effectC("", uiBase, zlInterface::multilingual::labels::bypass),
          sideC("", uiBase, zlInterface::multilingual::labels::externalSideChain),
          sgcC("", uiBase, zlInterface::multilingual::labels::staticGC),
          effectDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg,
                                                  BinaryData::fadpowerswitch_svgSize)),
          sideDrawable(juce::Drawable::createFromImageData(BinaryData::externalside_svg,
                                                           BinaryData::externalside_svgSize)),
          sgcDrawable(juce::Drawable::createFromImageData(BinaryData::staticgaincompensation_svg,
                                                          BinaryData::staticgaincompensation_svgSize)) {
        setInterceptsMouseClicks(false, true);
        addAndMakeVisible(logoPanel);
        addAndMakeVisible(fftSettingPanel);
        addAndMakeVisible(compSettingPanel);
        addAndMakeVisible(outputSettingPanel);
        addAndMakeVisible(conflictSettingPanel);
        addAndMakeVisible(generalSettingPanel);
        addAndMakeVisible(matchSettingPanel);

        effectC.setDrawable(effectDrawable.get());
        sideC.setDrawable(sideDrawable.get());
        sgcC.setDrawable(sgcDrawable.get());

        for (auto &c: {&effectC, &sideC, &sgcC}) {
            c->getLAF().enableShadow(false);
            c->getLAF().setShrinkScale(.0f);
            addAndMakeVisible(c);
            c->setBufferedToImage(true);
        }

        attach({
                   &effectC.getButton(),
                   &sideC.getButton(),
                   &sgcC.getButton(),
               },
               {zlDSP::effectON::ID, zlDSP::sideChain::ID, zlDSP::staticAutoGain::ID},
               p.parameters, buttonAttachments);

        sideC.getButton().onClick = [this]() {
            const auto isSideOn = static_cast<int>(sideC.getButton().getToggleState());
            const auto para = parametersNARef.getParameter(zlState::fftSideON::ID);
            para->beginChangeGesture();
            para->setValueNotifyingHost(zlState::fftSideON::convertTo01(isSideOn));
            para->endChangeGesture();
        };
    }

    void StatePanel::resized() {
        auto bound = getLocalBounds();
        const auto logoBound = bound.removeFromLeft(
            juce::roundToInt(static_cast<float>(bound.getWidth()) * .125f));
        logoPanel.setBounds(logoBound);

        const auto height = static_cast<float>(bound.getHeight());

        const auto effectBound = bound.removeFromRight(juce::roundToInt(height * .85f));
        effectC.setBounds(effectBound);

        const auto sideBound = bound.removeFromRight(juce::roundToInt(height * .85f));
        sideC.setBounds(sideBound);

        const auto sgcBound = bound.removeFromRight(juce::roundToInt(height * .85f));
        sgcC.setBounds(sgcBound);
        bound.removeFromRight(juce::roundToInt(height * .25f));

        bound.removeFromBottom(juce::roundToInt(uiBase.getFontSize() * .5f));

        const auto labelWidth = juce::roundToInt(height * labelSize);
        const auto gapWidth = juce::roundToInt(height * .5f);
        outputSettingPanel.setBounds(bound.removeFromRight(labelWidth));
        bound.removeFromRight(gapWidth);
        fftSettingPanel.setBounds(bound.removeFromRight(labelWidth));
        bound.removeFromRight(gapWidth);
        compSettingPanel.setBounds(bound.removeFromRight(labelWidth));
        bound.removeFromRight(gapWidth);
        conflictSettingPanel.setBounds(bound.removeFromRight(labelWidth));
        bound.removeFromRight(gapWidth);
        generalSettingPanel.setBounds(bound.removeFromRight(labelWidth));
        bound.removeFromRight(gapWidth);
        matchSettingPanel.setBounds(bound.removeFromRight(labelWidth));
    }
} // zlPanel
