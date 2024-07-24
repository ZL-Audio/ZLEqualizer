// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "PluginProcessor.hpp"
#include "BinaryData.h"
#include "panel/main_panel.hpp"
#include "state/state.hpp"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::AudioProcessorValueTreeState::Listener,
                     private juce::AsyncUpdater  {
public:
    explicit PluginEditor(PluginProcessor &);

    ~PluginEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;

    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor &processorRef;
    zlState::Property property;
    std::atomic<bool> isSizeChanged{false};

    zlPanel::MainPanel mainPanel;

    juce::Value lastUIWidth, lastUIHeight;
    constexpr const static std::array IDs{
        zlState::uiStyle::ID,
        zlState::windowW::ID,
        zlState::windowH::ID,
        "pre_r", "pre_g", "pre_b", "pre_o",
        "post_r", "post_g", "post_b", "post_o",
        "side_r", "side_g", "side_b", "side_o",
        "grid_r", "grid_g", "grid_b",
        zlState::wheelSensitivity::ID, zlState::wheelFineSensitivity::ID,
        zlState::rotaryStyle::ID, zlState::rotaryDragSensitivity::ID,
        zlState::refreshRate::ID,
        zlState::fftExtraTilt::ID, zlState::fftExtraSpeed::ID,
        zlState::singleCurveThickness::ID, zlState::sumCurveThickness::ID
    };

    void parameterChanged(const juce::String &parameterID, float newValue) override;

    void handleAsyncUpdate() override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
