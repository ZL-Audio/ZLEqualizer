// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../PluginProcessor.hpp"
#include "control_panel/control_panel.hpp"
#include "curve_panel/curve_panel.hpp"
#include "curve_panel/scale_panel.hpp"
#include "state_panel/state_panel.hpp"
#include "ui_setting_panel/ui_setting_panel.hpp"
#include "call_out_box/call_out_box.hpp"

namespace zlpanel {
    class MainPanel final : public juce::Component,
                            private juce::AudioProcessorValueTreeState::Listener,
                            private juce::AsyncUpdater {
    public:
        explicit MainPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~MainPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void parentHierarchyChanged() override;

        void lookAndFeelChanged() override;

        void repaintCallBack(const double nowT) {
            curvePanel.repaintCallBack(nowT);
        }

    private:
        PluginProcessor &processorRef;
        juce::AudioProcessorValueTreeState &state;
        zlgui::UIBase &uiBase;
        ControlPanel controlPanel;
        CurvePanel curvePanel;
        ScalePanel scalePanel;
        StatePanel statePanel;
        UISettingPanel uiSettingPanel;

        OutputBox outputBox;
        AnalyzerBox analyzerBox;
        DynamicBox dynamicBox;
        CollisionBox collisionBox;
        GeneralBox generalBox;

        zlgui::TooltipLookAndFeel tooltipLAF;
        zlgui::TooltipWindow tooltipWindow;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;

        void updateFFTs();
    };
}
