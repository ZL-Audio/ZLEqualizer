#include "PluginEditor.hpp"

PluginEditor::PluginEditor(PluginProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p), mainPanel(p.parameters, p.parameters) {
    juce::ignoreUnused(processorRef);

    setSize(232, 53);
    addAndMakeVisible(mainPanel);

    getConstrainer()->setFixedAspectRatio(float(232) / float(53));
    setResizable(true, p.wrapperType != PluginProcessor::wrapperType_AudioUnitv3);
}

PluginEditor::~PluginEditor() {
}

void PluginEditor::paint(juce::Graphics &g) {
    juce::ignoreUnused(g);
}

void PluginEditor::resized() {
    mainPanel.setBounds(getLocalBounds());
}
