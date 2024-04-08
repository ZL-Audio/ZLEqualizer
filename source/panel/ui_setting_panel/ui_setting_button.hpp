// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef UI_SETTING_BUTTON_HPP
#define UI_SETTING_BUTTON_HPP

#include "../../gui/gui.hpp"
#include "ui_setting_panel.hpp"

namespace zlPanel {

class UISettingButton final : public juce::Component {
public:
    explicit UISettingButton(UISettingPanel &panelToShow, zlInterface::UIBase &base);

    ~UISettingButton() override;

    void paint(juce::Graphics &g) override;

    void mouseDown(const juce::MouseEvent &event) override;

    void resized() override;

private:
    UISettingPanel &panel;
    zlInterface::UIBase &uiBase;
    juce::Label name;
    zlInterface::NameLookAndFeel nameLAF;
};

} // zlPanel

#endif //UI_SETTING_BUTTON_HPP
