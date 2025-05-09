// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"

namespace zlpanel {
    class ControlSettingPanel final : public juce::Component {
    public:
        static constexpr float heightP = 20.f;

        explicit ControlSettingPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~ControlSettingPanel() override;

        void loadSetting();

        void saveSetting();

        void resetSetting();

        void resized() override;

        void mouseDown(const juce::MouseEvent &event) override;

    private:
        PluginProcessor &pRef;
        zlgui::UIBase &uiBase;
        zlgui::NameLookAndFeel nameLAF;

        juce::Label wheelLabel;
        juce::Label dragLabel;
        std::array<zlgui::CompactLinearSlider, 4> sensitivitySliders;
        zlgui::CompactCombobox wheelReverseBox;
        juce::Label rotaryStyleLabel;
        zlgui::CompactCombobox rotaryStyleBox;
        zlgui::CompactLinearSlider rotaryDragSensitivitySlider;
        juce::Label sliderDoubleClickLabel;
        zlgui::CompactCombobox sliderDoubleClickBox;

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
} // zlpanel
