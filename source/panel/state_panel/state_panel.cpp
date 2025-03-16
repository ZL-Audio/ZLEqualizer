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
          outputValuePanel(p, base),
          outputSettingPanel(p, base, "", zlInterface::boxIdx::outputBox),
          analyzerSettingPanel(p, base, "Analyzer", zlInterface::boxIdx::analyzerBox),
          dynamicSettingPanel(p, base, "Dynamic", zlInterface::boxIdx::dynamicBox),

          collisionSettingPanel(p, base, "Collision", zlInterface::boxIdx::collisionBox),
          generalSettingPanel(p, base, "General", zlInterface::boxIdx::generalBox),
          matchSettingPanel(base),
          logoPanel(p, base, uiSettingPanel),
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

        addAndMakeVisible(outputSettingPanel);
        addAndMakeVisible(outputValuePanel);
        addAndMakeVisible(analyzerSettingPanel);
        addAndMakeVisible(dynamicSettingPanel);
        addAndMakeVisible(collisionSettingPanel);
        addAndMakeVisible(generalSettingPanel);
        addAndMakeVisible(matchSettingPanel);
        addAndMakeVisible(logoPanel);

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

        const auto buttonWidth = static_cast<int>(uiBase.getFontSize() * 2.5);
        const auto effectBound = bound.removeFromRight(buttonWidth);
        effectC.setBounds(effectBound);

        const auto sideBound = bound.removeFromRight(buttonWidth);
        sideC.setBounds(sideBound);

        const auto sgcBound = bound.removeFromRight(buttonWidth);
        sgcC.setBounds(sgcBound);

        bound.removeFromRight(buttonWidth / 4);

        bound.removeFromBottom(juce::roundToInt(uiBase.getFontSize() * .5f));

        const auto labelWidth = juce::roundToInt(height * labelSize);
        const auto gapWidth = juce::roundToInt(height * .5f);
        const auto mBound = bound.removeFromRight(labelWidth);
        outputValuePanel.setBounds(mBound);
        outputSettingPanel.setBounds(mBound);
        bound.removeFromRight(gapWidth);
        analyzerSettingPanel.setBounds(bound.removeFromRight(labelWidth));
        bound.removeFromRight(gapWidth);
        dynamicSettingPanel.setBounds(bound.removeFromRight(labelWidth));
        bound.removeFromRight(gapWidth);
        collisionSettingPanel.setBounds(bound.removeFromRight(labelWidth));
        bound.removeFromRight(gapWidth);
        generalSettingPanel.setBounds(bound.removeFromRight(labelWidth));
        bound.removeFromRight(gapWidth);
        matchSettingPanel.setBounds(bound.removeFromRight(labelWidth));
    }
} // zlPanel
