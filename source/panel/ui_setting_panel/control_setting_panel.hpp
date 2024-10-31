// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef CONTROL_UI_SETTING_PANEL_HPP
#define CONTROL_UI_SETTING_PANEL_HPP

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"

namespace zlPanel {

class ControlSettingPanel final : public juce::Component {
public:
    explicit ControlSettingPanel(PluginProcessor &p, zlInterface::UIBase &base);

    ~ControlSettingPanel() override;

    void loadSetting();

    void saveSetting();

    void resetSetting();

    void resized() override;

private:
    PluginProcessor &pRef;
    zlInterface::UIBase &uiBase;
    zlInterface::NameLookAndFeel nameLAF;

    juce::Label wheelLabel;
    juce::Label dragLabel;
    std::array<zlInterface::CompactLinearSlider, 4> sensitivitySliders;
    zlInterface::CompactCombobox wheelReverseBox;
    juce::Label rotaryStyleLabel;
    zlInterface::CompactCombobox rotaryStyleBox;
    zlInterface::CompactLinearSlider rotaryDragSensitivitySlider;
    juce::Label sliderDoubleClickLabel;
    zlInterface::CompactCombobox sliderDoubleClickBox;
};

} // zlPanel

#endif //CONTROL_UI_SETTING_PANEL_HPP
