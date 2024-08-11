// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ui_setting_button.hpp"

namespace zlPanel {
    UISettingButton::UISettingButton(UISettingPanel &panelToShow, zlInterface::UIBase &base)
        : panel(panelToShow), uiBase(base), nameLAF(base) {
        name.setText("UI", juce::sendNotification);
        nameLAF.setFontScale(1.375f);
        name.setLookAndFeel(&nameLAF);
        name.setEditable(false);
        name.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(name);
    }

    UISettingButton::~UISettingButton() {
        name.setLookAndFeel(nullptr);
    }

    void UISettingButton::paint(juce::Graphics &g) {
        g.setColour(uiBase.getBackgroundColor().withMultipliedAlpha(.25f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), uiBase.getFontSize() * .5f);
        g.setColour(uiBase.getTextColor().withMultipliedAlpha(.25f));
        juce::Path path;
        const auto bound = getLocalBounds().toFloat();
        path.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                 uiBase.getFontSize() * .5f, uiBase.getFontSize() * .5f,
                                 false, false, true, false);
        g.fillPath(path);
    }

    void UISettingButton::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        panel.loadSetting();
        panel.setVisible(true);
    }

    void UISettingButton::resized() {
        name.setBounds(getLocalBounds());
    }
} // zlPanel