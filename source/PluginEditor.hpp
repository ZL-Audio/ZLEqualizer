// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "PluginProcessor.hpp"
#include "BinaryData.h"
#include "panel/main_panel.hpp"
#include "state/state.hpp"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::Timer,
                     private juce::AudioProcessorValueTreeState::Listener,
                     private juce::AsyncUpdater {
public:
    explicit PluginEditor(PluginProcessor &);

    ~PluginEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;

    void resized() override;

    void visibilityChanged() override;

    void parentHierarchyChanged() override;

    void minimisationStateChanged(bool isNowMinimised) override;

private:
    PluginProcessor &processor_ref_;
    zlstate::Property &property_;
    std::atomic<bool> is_size_changed_{false};

    zlgui::UIBase ui_base_;
    zlpanel::MainPanel main_panel_;

    std::unique_ptr<juce::VBlankAttachment> vblank_;

    juce::Value last_ui_width_, last_ui_height_;
    static constexpr std::array kIDs{
        zlstate::windowW::ID,
        zlstate::windowH::ID,
        "text_r", "text_g", "text_b",
        "background_r", "background_g", "background_b",
        "shadow_r", "shadow_g", "shadow_b",
        "glow_r", "glow_g", "glow_b",
        "pre_r", "pre_g", "pre_b", "pre_o",
        "post_r", "post_g", "post_b", "post_o",
        "side_r", "side_g", "side_b", "side_o",
        "grid_r", "grid_g", "grid_b", "grid_o",
        "gain_r", "gain_g", "gain_b", "gain_o",
        "side_loudness_r", "side_loudness_g", "side_loudness_b", "side_loudness_o",
        zlstate::colourMap1Idx::ID, zlstate::colourMap2Idx::ID,
        zlstate::wheelSensitivity::ID, zlstate::wheelFineSensitivity::ID,
        zlstate::rotaryStyle::ID, zlstate::rotaryDragSensitivity::ID,
        zlstate::sliderDoubleClickFunc::ID,
        zlstate::refreshRate::ID,
        zlstate::fftExtraTilt::ID, zlstate::fftExtraSpeed::ID,
        zlstate::singleCurveThickness::ID, zlstate::sumCurveThickness::ID,
        zlstate::defaultPassFilterSlope::ID
    };

    void timerCallback() override;

    void parameterChanged(const juce::String &parameter_id, float new_value) override;

    void handleAsyncUpdate() override;

    void updateIsShowing();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
