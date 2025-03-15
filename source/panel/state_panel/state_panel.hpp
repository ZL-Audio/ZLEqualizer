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
#include "output_setting_panel.hpp"
#include "setting_panel.hpp"
#include "match_setting_panel.hpp"

namespace zlPanel {
    class StatePanel final : public juce::Component {
    public:
        explicit StatePanel(PluginProcessor &p,
                            zlInterface::UIBase &base,
                            UISettingPanel &uiSettingPanel);

        void resized() override;

    private:
        constexpr static float labelSize = 2.75f;
        zlInterface::UIBase &uiBase;
        juce::AudioProcessorValueTreeState &parametersNARef;

        OutputSettingPanel outputSettingPanel;
        SettingPanel analyzerSettingPanel;
        SettingPanel dynamicSettingPanel;
        SettingPanel collisionSettingPanel;
        SettingPanel generalSettingPanel;
        MatchSettingPanel matchSettingPanel;
        LogoPanel logoPanel;

        zlInterface::CompactButton effectC, sideC, sgcC;
        juce::OwnedArray<zlInterface::ButtonCusAttachment<true> > buttonAttachments{};
        const std::unique_ptr<juce::Drawable> effectDrawable;
        const std::unique_ptr<juce::Drawable> sideDrawable;
        const std::unique_ptr<juce::Drawable> sgcDrawable;
    };
} // zlPanel
