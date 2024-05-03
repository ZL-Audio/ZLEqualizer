// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "PluginEditor.hpp"

PluginEditor::PluginEditor(PluginProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p), property(p.state),
      mainPanel(p) {
    for (auto &ID: IDs) {
        processorRef.state.addParameterListener(ID, this);
    }
    // set font
    const auto sourceCodePro = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MiSansLatinMedium_ttf, BinaryData::MiSansLatinMedium_ttfSize);
    juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(sourceCodePro);

    // set size & size listener
    setResizeLimits(zlState::windowW::minV, zlState::windowH::minV,
                    zlState::windowW::maxV, zlState::windowH::maxV);
    getConstrainer()->setFixedAspectRatio(
        static_cast<float>(zlState::windowW::defaultV) / static_cast<float>(zlState::windowH::defaultV));
    setResizable(true, p.wrapperType != PluginProcessor::wrapperType_AudioUnitv3);
    lastUIWidth.referTo(p.state.getParameterAsValue(zlState::windowW::ID));
    lastUIHeight.referTo(p.state.getParameterAsValue(zlState::windowH::ID));
    setSize(lastUIWidth.getValue(), lastUIHeight.getValue());
    lastUIWidth.addListener(this);
    lastUIHeight.addListener(this);

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

void PluginEditor::valueChanged(juce::Value &) {
    setSize(lastUIWidth.getValue(), lastUIHeight.getValue());
}

void PluginEditor::parameterChanged(const juce::String &parameterID, float newValue) {
    juce::ignoreUnused(parameterID, newValue);
    triggerAsyncUpdate();
}

void PluginEditor::handleAsyncUpdate() {
    property.saveAPVTS(processorRef.state);
    sendLookAndFeelChange();
}