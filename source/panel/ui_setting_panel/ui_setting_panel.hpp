// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef UI_SETTING_PANEL_H
#define UI_SETTING_PANEL_H

#include "../../PluginProcessor.hpp"
#include "colour_setting_panel.hpp"
#include "control_setting_panel.hpp"
#include "other_ui_setting_panel.hpp"

namespace zlPanel {

class UISettingPanel final : public juce::Component {
public:
    explicit UISettingPanel(PluginProcessor &p, zlInterface::UIBase &base);

    ~UISettingPanel() override;

    void paint(juce::Graphics &g) override;

    void resized() override;

    void loadSetting();

    void mouseDown(const juce::MouseEvent &event) override;

    void visibilityChanged() override;

private:
    PluginProcessor &pRef;
    zlInterface::UIBase &uiBase;
    juce::Viewport viewPort;
    ColourSettingPanel colourPanel;
    ControlSettingPanel controlPanel;
    OtherUISettingPanel otherPanel;
    const std::unique_ptr<juce::Drawable> saveDrawable, closeDrawable, resetDrawable;
    zlInterface::ClickButton saveButton, closeButton, resetButton;

    zlInterface::NameLookAndFeel panelNameLAF;
    std::array<juce::Label, 3> panelLabels;

    juce::Label versionLabel;
    zlInterface::NameLookAndFeel labelLAF;

    enum panelIdx {
        colourP,
        controlP,
        otherP
    };
    panelIdx currentPanelIdx = colourP;

    void changeDisplayPanel();
};

} // zlPanel

#endif //UI_SETTING_PANEL_H
