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
#include "../multilingual/tooltip_helper.hpp"
#include "control_background.hpp"

namespace zlpanel {
    class SubLeftControlPanel final : public juce::Component {
    public:
        explicit SubLeftControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                     const multilingual::TooltipHelper& tooltip_helper);

        ~SubLeftControlPanel() override;

        int getIdealWidth() const;

        void resized() override;

        void repaintCallBackSlow();

        void updateBand();

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_;

        ControlBackground control_background_;

        const std::unique_ptr<juce::Drawable> close_drawable_;
        zlgui::button::ClickButton close_button_;

        const std::unique_ptr<juce::Drawable> bypass_drawable_;
        zlgui::button::ClickButton bypass_button_;

        zlgui::label::NameLookAndFeel label_laf_;

        juce::Label freq_label_;
        juce::Label gain_label_;
        juce::Label q_label_;

        const std::unique_ptr<juce::Drawable> left_drawable_;
        zlgui::button::ClickButton left_button_;

        juce::Label band_label_;

        const std::unique_ptr<juce::Drawable> right_drawable_;
        zlgui::button::ClickButton right_button_;

        zlgui::combobox::CompactCombobox ftype_box_;
        std::unique_ptr<zlgui::attachment::ComboBoxAttachment<true>> ftype_attachment_;

        zlgui::combobox::CompactCombobox slope_box_;
        std::unique_ptr<zlgui::attachment::ComboBoxAttachment<true>> slope_attachment_;

        zlgui::combobox::CompactCombobox stereo_box_;
        std::unique_ptr<zlgui::attachment::ComboBoxAttachment<true>> stereo_attachment_;

        zlgui::slider::TwoValueRotarySlider<true, false, false> q_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> q_attachment_;

        const std::unique_ptr<juce::Drawable> dynamic_drawable_;
        zlgui::button::ClickButton dynamic_button_;
        std::unique_ptr<zlgui::attachment::ButtonAttachment<true>> dynamic_attachment_;

        static constexpr std::array kDynamicResetIDs{
            zlp::PThreshold::kID, zlp::PKneeW::kID, zlp::PAttack::kID, zlp::PRelease::kID
        };

        template <bool is_right = true>
        size_t findClosestBand() const;

        void closeBand() const;

        void bypassBand() const;

        void turnOnOffDynamic() const;
    };
}
