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
#include "../../gui/gui.hpp"
#include "../helper/helper.hpp"
#include "../helper/freq_note.hpp"
#include "../multilingual/tooltip_helper.hpp"

#include "control_background.hpp"

namespace zlpanel {
    class RightControlPanel final : public juce::Component {
    public:
        explicit RightControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                   const multilingual::TooltipHelper& tooltip_helper);

        ~RightControlPanel() override;

        int getIdealWidth() const;

        void resized() override;

        void repaintCallBackSlow();

        void updateBand();

        void updateFreqMax(double freq_max);

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_;

        ControlBackground control_background_;

        const std::unique_ptr<juce::Drawable> bypass_drawable_;
        zlgui::button::ClickButton bypass_button_;
        std::unique_ptr<zlgui::attachment::ButtonAttachment<true>> bypass_attachment_;

        const std::unique_ptr<juce::Drawable> auto_drawable_;
        zlgui::button::ClickButton auto_button_;
        std::unique_ptr<zlgui::attachment::ButtonAttachment<true>> auto_attachment_;

        const std::unique_ptr<juce::Drawable> relative_drawable_;
        zlgui::button::ClickButton relative_button_;
        std::unique_ptr<zlgui::attachment::ButtonAttachment<true>> relative_attachment_;

        const std::unique_ptr<juce::Drawable> swap_drawable_;
        zlgui::button::ClickButton swap_button_;
        std::unique_ptr<zlgui::attachment::ButtonAttachment<true>> swap_attachment_;

        const std::unique_ptr<juce::Drawable> link_drawable_;
        zlgui::button::ClickButton link_button_;
        std::unique_ptr<zlgui::attachment::ButtonAttachment<true>> link_attachment_;

        zlgui::combobox::CompactCombobox ftype_box_;
        std::unique_ptr<zlgui::attachment::ComboBoxAttachment<true>> ftype_attachment_;

        zlgui::combobox::CompactCombobox slope_box_;
        std::unique_ptr<zlgui::attachment::ComboBoxAttachment<true>> slope_attachment_;

        zlgui::slider::CompactLinearSlider<true, true, true> th_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> th_attachment_;

        zlgui::slider::CompactLinearSlider<true, true, true> knee_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> knee_attachment_;

        zlgui::slider::CompactLinearSlider<true, true, true> attack_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> attack_attachment_;

        zlgui::slider::CompactLinearSlider<true, true, true> release_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> release_attachment_;

        zlgui::label::NameLookAndFeel label_laf_;

        juce::Label freq_label_;
        juce::Label q_label_;

        zlgui::slider::TwoValueRotarySlider<false, false, false> freq_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> freq_attachment_;
        double freq_max_{20000.0};

        zlgui::slider::TwoValueRotarySlider<false, false, false> q_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> q_attachment_;

        int c_side_ftype_{-1};

        void turnOnOffAuto();

        void mouseDown(const juce::MouseEvent& event) override;

        void mouseDrag(const juce::MouseEvent& event) override;

        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    };
}
