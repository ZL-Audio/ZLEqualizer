// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"
#include "../../multilingual/tooltip_helper.hpp"

#include "../../control_panel/control_background.hpp"

namespace zlpanel {
    class AnalyzerPanel final : public juce::Component,
                                private juce::ValueTree::Listener {
    public:
        explicit AnalyzerPanel(PluginProcessor& p, zlgui::UIBase& base,
                               const multilingual::TooltipHelper& tooltip_helper);

        ~AnalyzerPanel() override;

        int getIdealWidth() const;

        int getIdealHeight() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_;

        ControlBackground control_background_;

        zlgui::button::ClickTextButton pre_button_;
        zlgui::attachment::ButtonAttachment<true> pre_attach_;

        zlgui::button::ClickTextButton post_button_;
        zlgui::attachment::ButtonAttachment<true> post_attach_;

        zlgui::button::ClickTextButton side_button_;
        zlgui::attachment::ButtonAttachment<true> side_attach_;

        zlgui::combobox::CompactCombobox speed_box_;
        zlgui::attachment::ComboBoxAttachment<true> speed_attach_;

        zlgui::combobox::CompactCombobox slope_box_;
        zlgui::attachment::ComboBoxAttachment<true> slope_attach_;

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;
    };
}
