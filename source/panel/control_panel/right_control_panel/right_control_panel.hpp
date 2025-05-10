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
            explicit Background(zlgui::UIBase &base) : uiBase(base) {
                setBufferedToImage(true);
                setOpaque(true);
            }

            void paint(juce::Graphics &g) override {
                g.fillAll(uiBase.getBackgroundColor());
                const auto bound = getLocalBounds().toFloat();
                uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
            }

        private:
            zlgui::UIBase &uiBase;
        };

    public:
        explicit RightControlPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~RightControlPanel() override;

        void resized() override;

        void lookAndFeelChanged() override;

        void attachGroup(size_t idx);

    private:
        zlgui::UIBase &uiBase;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        std::atomic<bool> dynEditable{false};

        Background background;

        zlgui::CompactButton dynBypassC, dynSoloC, dynRelativeC, swapC;
        juce::OwnedArray<zlgui::ButtonCusAttachment<false> > buttonAttachments;

        zlgui::TwoValueRotarySlider<true, false> sideFreqC, sideQC;
        zlgui::CompactLinearSlider thresC, kneeC, attackC, releaseC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments;

        const std::unique_ptr<juce::Drawable> bypassDrawable, soloDrawable, relativeDrawable, swapDrawable;

        std::atomic<size_t> bandIdx;

        void updateMouseDragSensitivity();
    };
}
