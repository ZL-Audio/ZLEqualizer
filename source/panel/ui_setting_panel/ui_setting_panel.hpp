// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"

#include "colour_setting_panel.hpp"
#include "control_setting_panel.hpp"
#include "other_ui_setting_panel.hpp"
#include "credit_panel.hpp"
#include "ui_setting_components.hpp"
#include "../../gui/label/name_look_and_feel.hpp"
#include "../../gui/scrolling/scrollable_viewport.hpp"
#include "../background/panel_background.hpp"

namespace zlpanel {
    class UISettingPanel final : public juce::Component {
    public:
        explicit UISettingPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~UISettingPanel() override;

        void resized() override;

        void loadSetting();

        void flushPendingScroll();

        [[nodiscard]] int getIdealWidth() const;

        [[nodiscard]] int getIdealHeight() const;

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;

        ColourSettingPanel colour_panel_;
        ControlSettingPanel control_panel_;
        OtherUISettingPanel other_panel_;
        CreditPanel credit_panel_;

        PanelBackground background_;
        zlgui::label::NameLookAndFeel version_text_laf_;
        juce::Label version_text_;
        UISettingTabBar tab_bar_;
        zlgui::scrolling::ScrollableViewport view_port_;

        const std::unique_ptr<juce::Drawable> save_drawable_, close_drawable_, reset_drawable_, folder_open_drawable_;
        zlgui::button::ClickButton save_button_, close_button_, reset_button_, folder_open_button_;

        std::array<double, 4> view_positions_{};

        enum PanelIdx {
            kColourP,
            kControlP,
            kOtherP,
            kCreditP
        };

        PanelIdx current_panel_idx_ = kColourP;

        void changeDisplayPanel();

        void saveCurrentPanel();

        void resetCurrentPanel();

        void updateActionButtonStates();

        void lookAndFeelChanged() override;

        void visibilityChanged() override;
    };
}
