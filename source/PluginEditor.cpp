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
      processorRef(p),
      property(p.property),
      uiBase(p.state),
      mainPanel(p, uiBase) {
    for (auto &ID: IDs) {
        processorRef.state.addParameterListener(ID, this);
    }
    // set font
    const auto sourceCodePro = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MiSansLatinMedium_ttf, BinaryData::MiSansLatinMedium_ttfSize);
    juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(sourceCodePro);

    // set size & size listener
    setResizeLimits(static_cast<int>(zlstate::windowW::minV),
                    static_cast<int>(zlstate::windowH::minV),
                    static_cast<int>(zlstate::windowW::maxV),
                    static_cast<int>(zlstate::windowH::maxV));
    setResizable(true, p.wrapperType != PluginProcessor::wrapperType_AudioUnitv3);
    lastUIWidth.referTo(p.state.getParameterAsValue(zlstate::windowW::ID));
    lastUIHeight.referTo(p.state.getParameterAsValue(zlstate::windowH::ID));
    setSize(lastUIWidth.getValue(), lastUIHeight.getValue());

    // add main panel
    addAndMakeVisible(mainPanel);

    startTimerHz(2);

    updateIsShowing();
}

PluginEditor::~PluginEditor() {
    vblank.reset();
    for (auto &ID: IDs) {
        processorRef.state.removeParameterListener(ID, this);
    }

    stopTimer();
}

void PluginEditor::paint(juce::Graphics &g) {
    juce::ignoreUnused(g);
}

void PluginEditor::resized() {
    mainPanel.setBounds(getLocalBounds());
    lastUIWidth = getWidth();
    lastUIHeight = getHeight();
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

void PluginEditor::parameterChanged(const juce::String &parameterID, float newValue) {
    juce::ignoreUnused(parameterID, newValue);
    isSizeChanged.store(parameterID == zlstate::windowH::ID || parameterID == zlstate::windowW::ID);
    triggerAsyncUpdate();
}

void PluginEditor::handleAsyncUpdate() {
    property.saveAPVTS(processorRef.state);
    if (!isSizeChanged.exchange(false)) {
        sendLookAndFeelChange();
    }
}

void PluginEditor::timerCallback() {
    updateIsShowing();
}

void PluginEditor::updateIsShowing() {
    if (isShowing() != uiBase.getIsEditorShowing()) {
        uiBase.setIsEditorShowing(isShowing());
        if (uiBase.getIsEditorShowing()) {
            vblank = std::make_unique<juce::VBlankAttachment>(
                &mainPanel, [this](const double x) { mainPanel.repaintCallBack(x); });
        } else {
            vblank.reset();
        }
    }
}
