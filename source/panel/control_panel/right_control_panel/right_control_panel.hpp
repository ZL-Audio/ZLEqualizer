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
#include "../../panel_definitons.hpp"

namespace zlpanel {
    class RightControlPanel final : public juce::Component {
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
        explicit RightControlPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~RightControlPanel() override;

        void resized() override;

        void lookAndFeelChanged() override;

        void attachGroup(size_t idx);

    private:
        zlgui::UIBase &ui_base_;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        std::atomic<bool> dyn_editable_{false};

        Background background_;

        zlgui::CompactButton dyn_bypass_c_, dyn_solo_c_, dyn_relative_c_, swap_c_;
        juce::OwnedArray<zlgui::ButtonCusAttachment<false> > button_attachments_;

        zlgui::TwoValueRotarySlider<true, false> side_freq_c_, side_q_c_;
        zlgui::CompactLinearSlider threshold_c_, knee_c_, attack_c_, release_c_;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> slider_attachments_;

        const std::unique_ptr<juce::Drawable> bypass_drawable_, solo_drawable_, relative_drawable_, swap_drawable_;

        std::atomic<size_t> band_idx_;

        void updateMouseDragSensitivity();
    };
}
