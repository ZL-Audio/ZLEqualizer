// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef CONTROL_UI_SETTING_PANEL_HPP
#define CONTROL_UI_SETTING_PANEL_HPP

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"

namespace zlPanel {

class ControlSettingPanel final : public juce::Component {
public:
    static constexpr float heightP = 20.f;

    explicit ControlSettingPanel(PluginProcessor &p, zlInterface::UIBase &base);

    ~ControlSettingPanel() override;

    void loadSetting();

    void saveSetting();

    void resetSetting();

    void resized() override;

    void mouseDown(const juce::MouseEvent &event) override;

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

    juce::Label importLabel, exportLabel;
    std::unique_ptr<juce::FileChooser> myChooser;
    inline auto static const settingDirectory =
            juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("Audio")
            .getChildFile("Presets")
            .getChildFile(JucePlugin_Manufacturer)
            .getChildFile("Shared Settings");

    void importControls();

    void exportControls();
};

} // zlPanel

#endif //CONTROL_UI_SETTING_PANEL_HPP
