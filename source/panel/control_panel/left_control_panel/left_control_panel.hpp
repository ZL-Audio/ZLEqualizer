// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "BinaryData.h"

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../panel_definitons.hpp"
#include "../../helper/helper.hpp"
#include "reset_component.hpp"

namespace zlpanel {
    class LeftControlPanel final : public juce::Component,
                                   private juce::AudioProcessorValueTreeState::Listener,
                                   private juce::AsyncUpdater {
    private:
        class Background final : public juce::Component {
        public:
            explicit Background(zlgui::UIBase &base) : ui_base_(base) {
                setBufferedToImage(true);
                setOpaque(true);
            }

            void paint(juce::Graphics &g) override {
                g.fillAll(ui_base_.getBackgroundColor());
                const auto bound = getLocalBounds().toFloat();
                ui_base_.fillRoundedShadowRectangle(g, bound, 0.5f * ui_base_.getFontSize(), {.blur_radius = 0.25f});
            }

        private:
            zlgui::UIBase &ui_base_;
        };

    public:
        explicit LeftControlPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~LeftControlPanel() override;

        void resized() override;

        void lookAndFeelChanged() override;

        void attachGroup(size_t idx);

        zlgui::CompactButton &getDynamicAutoButton() { return dyn_l_c_; }

    private:
        PluginProcessor &processor_ref_;
        zlgui::UIBase &ui_base_;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;

        Background background_;

        zlgui::CompactButton bypass_c_, solo_c_, dyn_on_c_, dyn_l_c_;
        juce::OwnedArray<zlgui::ButtonCusAttachment<false> > button_attachments_;

        zlgui::CompactCombobox f_type_c_, slope_c_, stereo_c_;
        zlgui::LeftRightCombobox lr_box_;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> box_attachments_;

        zlgui::TwoValueRotarySlider<true, false> freq_c_;
        zlgui::TwoValueRotarySlider<true, true> gain_c_, q_c_;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> slider_attachments_;

        ResetComponent reset_component_;

        const std::unique_ptr<juce::Drawable> bypass_drawable_, solo_drawable_, dyn_on_drawable_, dyn_l_drawable_;

        std::atomic<size_t> band_idx_;

        std::atomic<bool> gain_c_editable_{true}, slop_c_enable_{true}, gain_s2_editable_{false}, q_s2_editable_{false};

        void parameterChanged(const juce::String &parameter_id, float new_value) override;

        void handleAsyncUpdate() override;

        void updateMouseDragSensitivity();
    };
}
