// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_setting_panel.hpp"

namespace zlPanel {
    MatchSettingPanel::MatchSettingPanel(zlInterface::UIBase &base)
        : uiBase(base),
          nameLAF(uiBase) {
        name.setText("Match", juce::sendNotification);
        nameLAF.setFontScale(1.375f);
        name.setLookAndFeel(&nameLAF);
        name.setEditable(false);
        name.setInterceptsMouseClicks(false, false);
        name.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(name);

        uiBase.setProperty(zlInterface::settingIdx::matchPanelShow, false);
        uiBase.setProperty(zlInterface::settingIdx::matchPanelFit, false);

        SettableTooltipClient::setTooltip(uiBase.getToolTipText(zlInterface::multilingual::labels::eqMatch));
        setBufferedToImage(true);
    }

    void MatchSettingPanel::resized() {
        name.setBounds(getLocalBounds());
    }

    void MatchSettingPanel::paint(juce::Graphics &g) {
        g.setColour(uiBase.getBackgroundColor().withMultipliedAlpha(.25f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), uiBase.getFontSize() * .5f);
        g.setColour(uiBase.getTextColor().withMultipliedAlpha(.25f));
        juce::Path path;
        const auto bound = getLocalBounds().toFloat();
        path.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                 uiBase.getFontSize() * .5f, uiBase.getFontSize() * .5f,
                                 false, false, true, true);
        g.fillPath(path);
    }

    void MatchSettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        const auto f = static_cast<bool>(uiBase.getProperty(zlInterface::settingIdx::matchPanelShow));
        uiBase.setProperty(zlInterface::settingIdx::matchPanelShow, !f);
        if (!f) {
            uiBase.setProperty(zlInterface::settingIdx::matchPanelFit, false);
        }
    }
} // zlPanel
