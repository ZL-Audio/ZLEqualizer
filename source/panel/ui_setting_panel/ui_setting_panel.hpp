// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_UISETTINGPANEL_H
#define ZLEqualizer_UISETTINGPANEL_H

#include "internal_setting_panel.hpp"
#include "../../PluginProcessor.hpp"

namespace zlPanel {

class UISettingPanel final : public juce::Component {
public:
    explicit UISettingPanel(PluginProcessor &p, zlInterface::UIBase &base);

    ~UISettingPanel() override;

    void paint(juce::Graphics &g) override;

    void resized() override;

    void visibilityChanged() override;

private:
    PluginProcessor &pRef;
    zlInterface::UIBase &uiBase;
    juce::Viewport viewPort;
    InternalSettingPanel internelPanel;
    const std::unique_ptr<juce::Drawable> saveDrawable, closeDrawable, resetDrawable;
    zlInterface::ClickButton saveButton, closeButton, resetButton;

    juce::Label versionLabel;
    zlInterface::NameLookAndFeel labelLAF;
};

} // zlPanel

#endif //ZLEqualizer_UISETTINGPANEL_H
