// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_MAIN_PANEL_HPP
#define ZLEqualizer_MAIN_PANEL_HPP

#include "../PluginProcessor.hpp"
#include "control_panel/control_panel.hpp"
#include "curve_panel/curve_panel.hpp"
#include "state_panel/state_panel.hpp"
#include "ui_setting_panel/ui_setting_panel.hpp"
#include "ui_setting_panel/ui_setting_button.hpp"

namespace zlPanel {
    class MainPanel final : public juce::Component,
                            private juce::AudioProcessorValueTreeState::Listener,
                            private juce::AsyncUpdater {
    public:
        explicit MainPanel(PluginProcessor &p);

        ~MainPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

    private:
        juce::AudioProcessorValueTreeState &state;
        zlInterface::UIBase uiBase;
        ControlPanel controlPanel;
        CurvePanel curvePanel;
        StatePanel statePanel;
        UISettingPanel uiSettingPanel;
        UISettingButton uiSettingButton;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;
    };
}


#endif //ZLEqualizer_MAIN_PANEL_HPP
