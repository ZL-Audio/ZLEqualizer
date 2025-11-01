// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "control_panel/control_panel.hpp"
#include "curve_panel/curve_panel.hpp"
#include "top_panel/top_panel.hpp"
#include "ui_setting_panel/ui_setting_panel.hpp"

namespace zlpanel {
    class MainPanel final : public juce::Component,
                            private juce::ValueTree::Listener,
                            private juce::Timer {
    public:
        explicit MainPanel(PluginProcessor& p, zlgui::UIBase& base);

        ~MainPanel() override;

        void resized() override;

        void repaintCallBack(double time_stamp);

        void startThreads();

        void stopThreads();

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        multilingual::TooltipHelper tooltip_helper_;

        RefreshHandler refresh_handler_;
        double previous_time_stamp_{-1.0};
        double refresh_rate_{-1.0};

        ControlPanel control_panel_;
        CurvePanel curve_panel_;
        TopPanel top_panel_;
        UISettingPanel ui_setting_panel_;

        zlgui::tooltip::TooltipLookAndFeel tooltip_laf_;
        zlgui::tooltip::TooltipWindow tooltip_window_;

        size_t c_band_{zlp::kBandNum};
        double c_sample_rate_{0.};

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;

        void timerCallback() override;

        void repaintCallBackSlow();
    };
} // zlpanel
