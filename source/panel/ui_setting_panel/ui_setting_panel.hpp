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
#include "colour_setting_panel.hpp"
#include "control_setting_panel.hpp"
#include "other_ui_setting_panel.hpp"

namespace zlpanel {
    class UISettingPanel final : public juce::Component {
    public:
        explicit UISettingPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~UISettingPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void loadSetting();

        void mouseDown(const juce::MouseEvent &event) override;

        void visibilityChanged() override;

        void setRendererList(const juce::StringArray &rendererList);

    private:
        PluginProcessor &processor_ref_;
        zlgui::UIBase &ui_base_;
        juce::Viewport view_port_;
        ColourSettingPanel colour_panel_;
        ControlSettingPanel control_panel_;
        OtherUISettingPanel other_panel_;
        const std::unique_ptr<juce::Drawable> save_drawable_, close_drawable_, reset_drawable_;
        zlgui::ClickButton save_button_, close_button_, reset_button_;

        zlgui::NameLookAndFeel panel_name_laf_;
        std::array<juce::Label, 3> panel_labels_;

        juce::Label version_label_;
        zlgui::NameLookAndFeel label_laf_;

        enum PanelIdx {
            kColourP,
            kControlP,
            kOtherP
        };

        PanelIdx current_panel_idx_ = kColourP;

        void changeDisplayPanel();
    };
} // zlpanel
