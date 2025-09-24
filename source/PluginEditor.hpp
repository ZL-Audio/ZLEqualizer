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

#include "state/state.hpp"
#include "gui/gui.hpp"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::Timer,
                     private juce::ValueTree::Listener,
                     private juce::AsyncUpdater {
public:
    explicit PluginEditor(PluginProcessor&);

    ~PluginEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;

    void resized() override;

    void visibilityChanged() override;

    void parentHierarchyChanged() override;

    void minimisationStateChanged(bool isNowMinimised) override;

private:
    PluginProcessor& p_ref_;
    zlstate::Property& property_;
    juce::Value last_ui_width_, last_ui_height_;

    zlgui::UIBase base_;

    std::unique_ptr<juce::VBlankAttachment> vblank_;

    void timerCallback() override;

    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;

    void handleAsyncUpdate() override;

    void updateIsShowing();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
