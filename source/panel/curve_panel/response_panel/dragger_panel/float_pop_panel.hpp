// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../../PluginProcessor.hpp"
#include "../../../../gui/gui.hpp"
#include "../../../helper/helper.hpp"
#include "../../../helper/freq_note.hpp"
#include "../../../multilingual/tooltip_helper.hpp"
#include "../../../control_panel/control_background.hpp"

namespace zlpanel {
    class FloatPopPanel final : public juce::Component,
                                private juce::ValueTree::Listener {
    public:
        explicit FloatPopPanel(PluginProcessor& p, zlgui::UIBase& base,
                               const multilingual::TooltipHelper& tooltip_helper);

        ~FloatPopPanel() override;

        void resized() override;

        void repaintCallBackSlow();

        void updateBand();

        int getIdealHeight() const;

        int getIdealWidth() const;

        void updatePosition(juce::Point<float> position);

        void setTargetVisible(bool is_target_visible);

        void updateFloatingBound(juce::Rectangle<float> bound);

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_{};

        ControlBackground control_background_;

        bool is_target_visible_{false};
        juce::Point<float> position_{};
        juce::Point<float> upper_center_{};
        juce::Point<float> lower_center_{};
        juce::Point<float> left_center_{};
        juce::Point<float> right_center_{};
        float ideal_height_{}, ideal_width_{};

        float x_min_{}, x_max_{}, x_mid_{}, y_min_{}, y_max_{};
        float y1_{}, y2_{}, y3_{};

        const std::unique_ptr<juce::Drawable> bypass_drawable_;
        zlgui::button::ClickButton bypass_button_;
        std::atomic<float>* filter_status_ptr_{nullptr};

        const std::unique_ptr<juce::Drawable> solo_drawable_;
        zlgui::button::ClickButton solo_button_;

        const std::unique_ptr<juce::Drawable> close_drawable_;
        zlgui::button::ClickButton close_button_;

        zlgui::combobox::CompactCombobox ftype_box_;
        std::unique_ptr<zlgui::attachment::ComboBoxAttachment<true>> ftype_attachment_;

        zlgui::combobox::CompactCombobox lr_box_;
        std::unique_ptr<zlgui::attachment::ComboBoxAttachment<true>> lr_attachment_;

        zlgui::slider::CompactLinearSlider<false, false, false> freq_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> freq_attachment_;

        void updateTransformation();

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
    };
}
