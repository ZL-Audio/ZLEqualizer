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
#include "../ui_setting_panel/ui_setting_panel.hpp"

namespace zlPanel {
    class LogoPanel final : public juce::Component, public juce::SettableTooltipClient {
    public:
        explicit LogoPanel(PluginProcessor &p,
                           zlInterface::UIBase &base,
                           UISettingPanel &uiSettingPanel);

        void paint(juce::Graphics &g) override;

        void setJustification(int justificationFlags);

        void mouseDoubleClick(const juce::MouseEvent &event) override;

    private:
        juce::AudioProcessorValueTreeState &stateRef;
        zlInterface::UIBase &uiBase;
        UISettingPanel &panelToShow;
        const std::unique_ptr<juce::Drawable> brandDrawable, logoDrawable;
        juce::Justification justification{juce::Justification::topLeft};
    };
} // zlPanel
