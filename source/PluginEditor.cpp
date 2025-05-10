// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "PluginEditor.hpp"

PluginEditor::PluginEditor(PluginProcessor &p)
    : AudioProcessorEditor(&p),
      processor_ref_(p),
      property_(p.property_),
      ui_base_(p.state_),
      main_panel_(p, ui_base_) {
    for (auto &ID: kIDs) {
        processor_ref_.state_.addParameterListener(ID, this);
    }
    // set font
    const auto font_face = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MiSansLatinMedium_ttf, BinaryData::MiSansLatinMedium_ttfSize);
    juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(font_face);

    // set size & size listener
    setResizeLimits(static_cast<int>(zlstate::windowW::minV),
                    static_cast<int>(zlstate::windowH::minV),
                    static_cast<int>(zlstate::windowW::maxV),
                    static_cast<int>(zlstate::windowH::maxV));
    setResizable(true, p.wrapperType != PluginProcessor::wrapperType_AudioUnitv3);
    last_ui_width_.referTo(p.state_.getParameterAsValue(zlstate::windowW::ID));
    last_ui_height_.referTo(p.state_.getParameterAsValue(zlstate::windowH::ID));
    setSize(last_ui_width_.getValue(), last_ui_height_.getValue());

    // add the main panel
    addAndMakeVisible(main_panel_);

    startTimerHz(2);

    updateIsShowing();
}

PluginEditor::~PluginEditor() {
    vblank_.reset();
    for (auto &id: kIDs) {
        processor_ref_.state_.removeParameterListener(id, this);
    }

    stopTimer();
}

void PluginEditor::paint(juce::Graphics &g) {
    juce::ignoreUnused(g);
}

void PluginEditor::resized() {
    main_panel_.setBounds(getLocalBounds());
    last_ui_width_ = getWidth();
    last_ui_height_ = getHeight();
}

void PluginEditor::visibilityChanged() {
    updateIsShowing();
}

void PluginEditor::parentHierarchyChanged() {
    updateIsShowing();
}

void PluginEditor::minimisationStateChanged(bool) {
    updateIsShowing();
}

void PluginEditor::parameterChanged(const juce::String &parameter_id, float new_value) {
    juce::ignoreUnused(parameter_id, new_value);
    is_size_changed_.store(parameter_id == zlstate::windowH::ID || parameter_id == zlstate::windowW::ID);
    triggerAsyncUpdate();
}

void PluginEditor::handleAsyncUpdate() {
    property_.saveAPVTS(processor_ref_.state_);
    if (!is_size_changed_.exchange(false)) {
        sendLookAndFeelChange();
    }
}

void PluginEditor::timerCallback() {
    updateIsShowing();
}

void PluginEditor::updateIsShowing() {
    if (isShowing() != ui_base_.getIsEditorShowing()) {
        ui_base_.setIsEditorShowing(isShowing());
        if (ui_base_.getIsEditorShowing()) {
            vblank_ = std::make_unique<juce::VBlankAttachment>(
                &main_panel_, [this](const double x) { main_panel_.repaintCallBack(x); });
        } else {
            vblank_.reset();
        }
    }
}
