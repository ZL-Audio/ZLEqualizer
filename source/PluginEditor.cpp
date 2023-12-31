#include "PluginEditor.hpp"

PluginEditor::PluginEditor(PluginProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p), mainPanel(p) {
    juce::ignoreUnused(processorRef);

    auto sourceCodePro = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MiSansLatinMedium_ttf,
        BinaryData::MiSansLatinMedium_ttfSize);
    juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(sourceCodePro);

    constexpr auto width = 800, height = 500;
    setSize(width, height);
    addAndMakeVisible(mainPanel);

    getConstrainer()->setFixedAspectRatio(float(width) / float(height));
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
