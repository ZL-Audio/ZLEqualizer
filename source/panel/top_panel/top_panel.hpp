// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "logo_panel.hpp"
#include "output_label.hpp"

namespace zlpanel {
    class TopPanel final : public juce::Component {
    public:
        explicit TopPanel(PluginProcessor& p, zlgui::UIBase& base,
                          multilingual::TooltipHelper& tooltip_helper);

        void paint(juce::Graphics& g) override;

        int getIdealHeight() const;

        void resized() override;

        void repaintCallbackSlow();

    private:
        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_;
        LogoPanel logo_panel_;
        OutputLabel output_label_;

        zlgui::combobox::CompactCombobox fstruct_box_;
        zlgui::attachment::ComboBoxAttachment<true> fstruct_attach_;

        const std::unique_ptr<juce::Drawable> bypass_drawable_;
        zlgui::button::ClickButton bypass_button_;
        zlgui::attachment::ButtonAttachment<true> bypass_attach_;

        const std::unique_ptr<juce::Drawable> ext_drawable_;
        zlgui::button::ClickButton ext_button_;
        zlgui::attachment::ButtonAttachment<true> ext_attach_;
    };
}
