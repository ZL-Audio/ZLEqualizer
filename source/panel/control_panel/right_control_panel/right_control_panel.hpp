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

namespace zlPanel {
    class RightControlPanel final : public juce::Component,
                                    private juce::AudioProcessorValueTreeState::Listener,
                                    private juce::AsyncUpdater {
    public:
        explicit RightControlPanel(PluginProcessor &p, zlInterface::UIBase &base);

        ~RightControlPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void lookAndFeelChanged() override;

        void attachGroup(size_t idx);

    private:
        zlInterface::UIBase &uiBase;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        std::atomic<bool> dynEditable{false};

        zlInterface::CompactButton dynBypassC, dynSoloC, dynRelativeC, sideChainC;
        juce::OwnedArray<zlInterface::ButtonCusAttachment<false> > buttonAttachments;

        zlInterface::TwoValueRotarySlider<true, false> sideFreqC, sideQC;
        zlInterface::CompactLinearSlider thresC, kneeC, attackC, releaseC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments;

        const std::unique_ptr<juce::Drawable> bypassDrawable, soloDrawable, relativeDrawable, sideDrawable;

        std::atomic<size_t> bandIdx;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;

        void updateMouseDragSensitivity();
    };
}
