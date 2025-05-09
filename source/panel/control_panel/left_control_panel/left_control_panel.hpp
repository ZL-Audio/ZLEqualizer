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
        explicit LeftControlPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~LeftControlPanel() override;

        void resized() override;

        void lookAndFeelChanged() override;

        void attachGroup(size_t idx);

        zlgui::CompactButton &getDynamicAutoButton() { return dynLC; }

    private:
        PluginProcessor &processor_ref;
        zlgui::UIBase &uiBase;
        juce::AudioProcessorValueTreeState &parameters_ref, &parameters_NA_ref;

        Background background;

        zlgui::CompactButton bypassC, soloC, dynONC, dynLC;
        juce::OwnedArray<zlgui::ButtonCusAttachment<false> > buttonAttachments;

        zlgui::CompactCombobox fTypeC, slopeC, stereoC;
        zlgui::LeftRightCombobox lrBox;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments;

        zlgui::TwoValueRotarySlider<true, false> freqC;
        zlgui::TwoValueRotarySlider<true, true> gainC, qC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments;

        ResetComponent resetComponent;

        const std::unique_ptr<juce::Drawable> bypassDrawable, soloDrawable, dynONDrawable, dynLeDrawable;

        std::atomic<size_t> bandIdx;

        std::atomic<bool> gainCEditable{true}, slopCEnable{true}, gainS2Editable{false}, qS2Editable{false};

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;

        void updateMouseDragSensitivity();
    };
}
