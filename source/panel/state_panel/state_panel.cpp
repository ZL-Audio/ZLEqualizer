// Copyright (C) 2024 - zsliu98
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
        : uiBase(base),
          logoPanel(p, base, uiSettingPanel),
          fftSettingPanel(p, base),
          compSettingPanel(p, base),
          outputSettingPanel(p, base),
          conflictSettingPanel(p, base),
          generalSettingPanel(p, base),
          effectC("all", uiBase),
          sgcC("S", uiBase),
          effectDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg,
                                                  BinaryData::fadpowerswitch_svgSize)) {
        setInterceptsMouseClicks(false, true);
        addAndMakeVisible(logoPanel);
        addAndMakeVisible(fftSettingPanel);
        addAndMakeVisible(compSettingPanel);
        addAndMakeVisible(outputSettingPanel);
        addAndMakeVisible(conflictSettingPanel);
        addAndMakeVisible(generalSettingPanel);

        effectC.setDrawable(effectDrawable.get());

        for (auto &c: {&effectC, &sgcC}) {
            c->getLAF().setLabelScale(1.7f);
            c->getLAF().enableShadow(false);
            c->getLAF().setShrinkScale(.0f);
            addAndMakeVisible(c);
        }

        addAndMakeVisible(effectC);
        addAndMakeVisible(sgcC);

        attach({
                       &effectC.getButton(),
                       &sgcC.getButton(),
                   },
                   {zlDSP::effectON::ID, zlDSP::staticAutoGain::ID},
                   p.parameters, buttonAttachments);
    }

    void StatePanel::resized() {
        auto bound = getLocalBounds().toFloat();
        const auto logoBound = bound.removeFromLeft(bound.getWidth() * .125f);
        logoPanel.setBounds(logoBound.toNearestInt());
        const auto height = bound.getHeight();

        const auto effectBound = bound.removeFromRight(height * .8f);
        effectC.setBounds(effectBound.toNearestInt());
        bound.removeFromRight(height * .15f);

        const auto sgcBound = bound.removeFromRight(height * .75f);
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
    }
} // zlPanel
