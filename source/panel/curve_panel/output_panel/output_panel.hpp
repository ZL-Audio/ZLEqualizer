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
    class OutputPanel final : public juce::Component,
                              private juce::ValueTree::Listener {
    public:
        explicit OutputPanel(PluginProcessor& p, zlgui::UIBase& base,
                             const multilingual::TooltipHelper& tooltip_helper);

        ~OutputPanel() override;

        int getIdealWidth() const;

        int getIdealHeight() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_;

        ControlBackground control_background_;

        zlgui::label::NameLookAndFeel name_laf_;
        juce::Label gain_label_;
        juce::Label scale_label_;

        zlgui::slider::TwoValueRotarySlider<false, false, false> gain_slider_;
        zlgui::attachment::SliderAttachment<true> gain_attach_;

        zlgui::slider::TwoValueRotarySlider<false, false, false> scale_slider_;
        zlgui::attachment::SliderAttachment<true> scale_attach_;

        const std::unique_ptr<juce::Drawable> sgc_drawable_;
        zlgui::button::ClickButton sgc_button_;
        zlgui::attachment::ButtonAttachment<true> sgc_attach_;

        const std::unique_ptr<juce::Drawable> lm_drawable_;
        zlgui::button::ClickButton lm_button_;

        const std::unique_ptr<juce::Drawable> agc_drawable_;
        zlgui::button::ClickButton agc_button_;
        zlgui::attachment::ButtonAttachment<true> agc_attach_;

        const std::unique_ptr<juce::Drawable> phase_drawable_;
        zlgui::button::ClickButton phase_button_;
        zlgui::attachment::ButtonAttachment<true> phase_attach_;

        juce::Label lookahead_label_;
        zlgui::slider::CompactLinearSlider<false, false, false> lookahead_slider_;
        zlgui::attachment::SliderAttachment<true> lookahead_attach_;

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;
    };
}
