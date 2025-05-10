// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "state_panel.hpp"

namespace zlpanel {
    StatePanel::StatePanel(PluginProcessor &p,
                           zlgui::UIBase &base,
                           UISettingPanel &uiSettingPanel)
        : ui_base_(base), parameters_NA_ref_(p.parameters_NA),
          outputValuePanel(p, base),
          outputSettingPanel(p, base, "", zlgui::BoxIdx::kOutputBox),
          analyzerSettingPanel(p, base, "Analyzer", zlgui::BoxIdx::kAnalyzerBox),
          dynamicSettingPanel(p, base, "Dynamic", zlgui::BoxIdx::kDynamicBox),

          collisionSettingPanel(p, base, "Collision", zlgui::BoxIdx::kCollisionBox),
          generalSettingPanel(p, base, "General", zlgui::BoxIdx::kGeneralBox),
          matchSettingPanel(base),
          logoPanel(p, base, uiSettingPanel),
          effectC("", ui_base_, zlgui::multilingual::Labels::kBypass),
          sideC("", ui_base_, zlgui::multilingual::Labels::kExternalSideChain),
          sgcC("", ui_base_, zlgui::multilingual::Labels::kStaticGC),
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
               {zlp::effectON::ID, zlp::sideChain::ID, zlp::staticAutoGain::ID},
               p.parameters, buttonAttachments);

        sideC.getButton().onClick = [this]() {
            const auto isSideOn = static_cast<int>(sideC.getButton().getToggleState());
            const auto para = parameters_NA_ref_.getParameter(zlstate::fftSideON::ID);
            para->beginChangeGesture();
            para->setValueNotifyingHost(zlstate::fftSideON::convertTo01(isSideOn));
            para->endChangeGesture();
        };
    }

    void StatePanel::resized() {
        auto bound = getLocalBounds();
        const auto logoBound = bound.removeFromLeft(
            juce::roundToInt(static_cast<float>(bound.getWidth()) * .125f));
        logoPanel.setBounds(logoBound);

        const auto height = static_cast<float>(bound.getHeight());

        const auto buttonWidth = static_cast<int>(ui_base_.getFontSize() * 2.5);
        const auto effectBound = bound.removeFromRight(buttonWidth);
        effectC.setBounds(effectBound);

        const auto sideBound = bound.removeFromRight(buttonWidth);
        sideC.setBounds(sideBound);

        const auto sgcBound = bound.removeFromRight(buttonWidth);
        sgcC.setBounds(sgcBound);

        bound.removeFromRight(buttonWidth / 4);

        bound.removeFromBottom(juce::roundToInt(ui_base_.getFontSize() * .5f));

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
} // zlpanel
