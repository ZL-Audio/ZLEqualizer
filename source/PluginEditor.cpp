// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "PluginEditor.hpp"

PluginEditor::PluginEditor(PluginProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p), property(p.property), mainPanel(p) {
    for (auto &ID: IDs) {
        processorRef.state.addParameterListener(ID, this);
    }
    // set font
    const auto sourceCodePro = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MiSansLatinMedium_ttf, BinaryData::MiSansLatinMedium_ttfSize);
    juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(sourceCodePro);

    // set size & size listener
    setResizeLimits(static_cast<int>(zlState::windowW::minV),
                    static_cast<int>(zlState::windowH::minV),
                    static_cast<int>(zlState::windowW::maxV),
                    static_cast<int>(zlState::windowH::maxV));
    setResizable(true, p.wrapperType != PluginProcessor::wrapperType_AudioUnitv3);
    lastUIWidth.referTo(p.state.getParameterAsValue(zlState::windowW::ID));
    lastUIHeight.referTo(p.state.getParameterAsValue(zlState::windowH::ID));
    setSize(lastUIWidth.getValue(), lastUIHeight.getValue());

    // add main panel
    addAndMakeVisible(mainPanel);
}

PluginEditor::~PluginEditor() {
    for (auto &ID: IDs) {
        processorRef.state.removeParameterListener(ID, this);
    }
}

void PluginEditor::paint(juce::Graphics &g) {
    juce::ignoreUnused(g);
}

void PluginEditor::resized() {
    mainPanel.setBounds(getLocalBounds());
    lastUIWidth = getWidth();
    lastUIHeight = getHeight();
}

void PluginEditor::parameterChanged(const juce::String &parameterID, float newValue) {
    juce::ignoreUnused(parameterID, newValue);
    isSizeChanged.store(parameterID == zlState::windowH::ID || parameterID == zlState::windowW::ID);
    triggerAsyncUpdate();
}

void PluginEditor::handleAsyncUpdate() {
    property.saveAPVTS(processorRef.state);
    if (!isSizeChanged.exchange(false)) {
        sendLookAndFeelChange();
    }
}
