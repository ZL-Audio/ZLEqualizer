// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_setting_panel.hpp"

namespace zlpanel {
    MatchSettingPanel::MatchSettingPanel(zlgui::UIBase &base)
        : ui_base_(base) {

        ui_base_.setProperty(zlgui::SettingIdx::kMatchPanelShow, false);
        ui_base_.setProperty(zlgui::SettingIdx::kMatchPanelFit, false);

        SettableTooltipClient::setTooltip(ui_base_.getToolTipText(zlgui::multilingual::Labels::kEQMatch));
        setBufferedToImage(true);

        ui_base_.getValueTree().addListener(this);
    }

    MatchSettingPanel::~MatchSettingPanel() {
        ui_base_.getValueTree().removeListener(this);
    }

    void MatchSettingPanel::paint(juce::Graphics &g) {
        const auto is_match_show = static_cast<bool>(ui_base_.getProperty(zlgui::SettingIdx::kMatchPanelShow));
        if (is_match_show) {
            g.setColour(ui_base_.getTextColor().withMultipliedAlpha(.25f));
        } else {
            g.setColour(ui_base_.getTextColor().withMultipliedAlpha(.125f));
        }

        juce::Path path;
        const auto bound = getLocalBounds().toFloat();
        path.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                 ui_base_.getFontSize() * .5f, ui_base_.getFontSize() * .5f,
                                 false, false, true, true);
        g.fillPath(path);
        g.setFont(ui_base_.getFontSize() * 1.375f);
        if (is_match_show) {
            g.setColour(ui_base_.getTextColor());
        } else {
            g.setColour(ui_base_.getTextColor().withAlpha(.75f));
        }
        g.drawText("Match", bound, juce::Justification::centred);
    }

    void MatchSettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        const auto f = static_cast<bool>(ui_base_.getProperty(zlgui::SettingIdx::kMatchPanelShow));
        ui_base_.setProperty(zlgui::SettingIdx::kMatchPanelShow, !f);
        if (!f) {
            ui_base_.setProperty(zlgui::SettingIdx::kMatchPanelFit, false);
        }
    }

    void MatchSettingPanel::mouseEnter(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        ui_base_.closeAllBox();
    }

    void MatchSettingPanel::valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                                     const juce::Identifier &property) {
        juce::ignoreUnused(tree_whose_property_has_changed);
        if (zlgui::UIBase::isProperty(zlgui::SettingIdx::kMatchPanelShow, property)) {
            repaint();
        }
    }
} // zlpanel
