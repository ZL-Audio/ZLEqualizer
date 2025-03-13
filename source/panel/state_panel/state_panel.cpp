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
        auto bound = getLocalBounds().toFloat();
        const auto logoBound = bound.removeFromLeft(bound.getWidth() * .125f);
        logoPanel.setBounds(logoBound.toNearestInt());
        const auto height = bound.getHeight();

        const auto effectBound = bound.removeFromRight(height * .85f);
        effectC.setBounds(effectBound.toNearestInt());

        const auto sideBound = bound.removeFromRight(height * .85f);
        sideC.setBounds(sideBound.toNearestInt());

        const auto sgcBound = bound.removeFromRight(height * .85f);
        sgcC.setBounds(sgcBound.toNearestInt());
        bound.removeFromRight(height * .25f);

        bound.removeFromBottom(uiBase.getFontSize() * .5f);

        const auto outputSettingBound = bound.removeFromRight(height * labelSize);
        outputSettingPanel.setBounds(outputSettingBound.toNearestInt());
        bound.removeFromRight(height * .5f);
        const auto fftSettingBound = bound.removeFromRight(height * labelSize);
        fftSettingPanel.setBounds(fftSettingBound.toNearestInt());
        bound.removeFromRight(height * .5f);
        const auto compSettingBound = bound.removeFromRight(height * labelSize);
        compSettingPanel.setBounds(compSettingBound.toNearestInt());
        bound.removeFromRight(height * .5f);
        const auto conflictSettingBound = bound.removeFromRight(height * labelSize);
        conflictSettingPanel.setBounds(conflictSettingBound.toNearestInt());
        bound.removeFromRight(height * .5f);
        const auto generalSettingBound = bound.removeFromRight(height * labelSize);
        generalSettingPanel.setBounds(generalSettingBound.toNearestInt());
        bound.removeFromRight(height * .5f);
        const auto matchSettingBound = bound.removeFromRight(height * labelSize);
        matchSettingPanel.setBounds(matchSettingBound.toNearestInt());
    }
} // zlPanel
