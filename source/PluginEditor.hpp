// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "PluginProcessor.hpp"

#include "panel/main_panel.hpp"
#include "gui/gui.hpp"
#include "state/state.hpp"

//==============================================================================
class PluginEditor final : public juce::AudioProcessorEditor,
                           private juce::MultiTimer,
                           private juce::ValueTree::Listener,
                           private juce::AsyncUpdater {
public:
    explicit PluginEditor(PluginProcessor& p);

    ~PluginEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;

    void resized() override;

    void visibilityChanged() override;

    void parentHierarchyChanged() override;

    void minimisationStateChanged(bool isNowMinimised) override;

    int getControlParameterIndex(Component& c) override;

    void mouseDown(const juce::MouseEvent& event) override;

private:
    PluginProcessor& p_ref_;
    zlstate::DummyProcessor dummy_processor_;
    juce::AudioProcessorValueTreeState state_;
    zlstate::Property property_;
    juce::Value last_ui_width_, last_ui_height_;

    zlgui::UIBase base_;
    zlpanel::MainPanel main_panel_;

    std::unique_ptr<juce::VBlankAttachment> vblank_;

    static constexpr int kVisibilityTimer = 0;
    static constexpr int kPropertySaveTimer = 1;

    static constexpr int kPropertySaveDelayMS = 1000;

    void timerCallback(int timer_id) override;

    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;

    void handleAsyncUpdate() override;

    void updateIsShowing();

    void schedulePropertySave();

    void flushPendingPropertySave();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
