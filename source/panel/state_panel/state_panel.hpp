// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../ui_setting_panel/ui_setting_panel.hpp"
#include "logo_panel.hpp"
#include "output_value_panel.hpp"
#include "setting_panel.hpp"
#include "match_setting_panel.hpp"

namespace zlpanel {
    class StatePanel final : public juce::Component {
    public:
        explicit StatePanel(PluginProcessor &p,
                            zlgui::UIBase &base,
                            UISettingPanel &uiSettingPanel);

        void resized() override;

    private:
        constexpr static float kLabelSize = 2.75f;
        zlgui::UIBase &ui_base_;
        juce::AudioProcessorValueTreeState &parameters_NA_ref_;

        OutputValuePanel output_value_panel_;
        SettingPanel output_setting_panel_;
        SettingPanel analyzer_setting_panel_;
        SettingPanel dynamic_setting_panel_;
        SettingPanel collision_setting_panel_;
        SettingPanel general_setting_panel_;
        MatchSettingPanel match_setting_panel_;
        LogoPanel logo_panel_;

        zlgui::CompactButton effect_c_, side_c_, sgc_c_;
        juce::OwnedArray<zlgui::ButtonCusAttachment<true> > button_attachments_{};
        const std::unique_ptr<juce::Drawable> effect_drawable_;
        const std::unique_ptr<juce::Drawable> side_drawable_;
        const std::unique_ptr<juce::Drawable> sgc_drawable_;
    };
} // zlpanel
