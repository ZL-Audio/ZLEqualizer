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
        : uiBase(base) {

        uiBase.setProperty(zlInterface::settingIdx::matchPanelShow, false);
        uiBase.setProperty(zlInterface::settingIdx::matchPanelFit, false);

        SettableTooltipClient::setTooltip(uiBase.getToolTipText(zlInterface::multilingual::labels::eqMatch));
        setBufferedToImage(true);

        uiBase.getValueTree().addListener(this);
    }

    MatchSettingPanel::~MatchSettingPanel() {
        uiBase.getValueTree().removeListener(this);
    }

    void MatchSettingPanel::paint(juce::Graphics &g) {
        const auto isMatchShow = static_cast<bool>(uiBase.getProperty(zlInterface::settingIdx::matchPanelShow));
        if (isMatchShow) {
            g.setColour(uiBase.getTextColor().withMultipliedAlpha(.25f));
        } else {
            g.setColour(uiBase.getTextColor().withMultipliedAlpha(.125f));
        }

        juce::Path path;
        const auto bound = getLocalBounds().toFloat();
        path.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                 uiBase.getFontSize() * .5f, uiBase.getFontSize() * .5f,
                                 false, false, true, true);
        g.fillPath(path);
        g.setFont(uiBase.getFontSize() * 1.375f);
        if (isMatchShow) {
            g.setColour(uiBase.getTextColor());
        } else {
            g.setColour(uiBase.getTextColor().withAlpha(.75f));
        }
        g.drawText("Match", bound, juce::Justification::centred);
    }

    void MatchSettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        const auto f = static_cast<bool>(uiBase.getProperty(zlInterface::settingIdx::matchPanelShow));
        uiBase.setProperty(zlInterface::settingIdx::matchPanelShow, !f);
        if (!f) {
            uiBase.setProperty(zlInterface::settingIdx::matchPanelFit, false);
        }
    }

    void MatchSettingPanel::mouseEnter(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        uiBase.closeAllBox();
    }

    void MatchSettingPanel::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                                     const juce::Identifier &property) {
        juce::ignoreUnused(treeWhosePropertyHasChanged);
        if (uiBase.isProperty(zlInterface::settingIdx::matchPanelShow, property)) {
            repaint();
        }
    }
} // zlPanel
