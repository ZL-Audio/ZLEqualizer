// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_MAIN_PANEL_HPP
#define ZLEqualizer_MAIN_PANEL_HPP

#include "../PluginProcessor.hpp"
#include "control_panel/control_panel.hpp"
#include "curve_panel/curve_panel.hpp"
#include "curve_panel/scale_panel.hpp"
#include "state_panel/state_panel.hpp"
#include "ui_setting_panel/ui_setting_panel.hpp"

namespace zlPanel {
    class MainPanel final : public juce::Component,
                            private juce::AudioProcessorValueTreeState::Listener,
                            private juce::AsyncUpdater {
    public:
        explicit MainPanel(PluginProcessor &p);

        ~MainPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void parentHierarchyChanged() override;

    private:
        PluginProcessor &processorRef;
        juce::AudioProcessorValueTreeState &state;
        zlInterface::UIBase uiBase;
        ControlPanel controlPanel;
        CurvePanel curvePanel;
        ScalePanel scalePanel;
        StatePanel statePanel;
        UISettingPanel uiSettingPanel;

        zlInterface::TooltipLookAndFeel tooltipLAF;
        juce::TooltipWindow tooltipWindow;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;

        void updateFFTs();
    };
}


#endif //ZLEqualizer_MAIN_PANEL_HPP
