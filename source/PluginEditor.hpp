#pragma once

#include "PluginProcessor.hpp"
#include "BinaryData.h"
#include "panel/main_panel.hpp"
#include "state/state.hpp"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::Value::Listener,
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

    zlPanel::MainPanel mainPanel;

    juce::Value lastUIWidth, lastUIHeight;
    constexpr const static std::array IDs{
        zlState::uiStyle::ID,
        zlState::windowW::ID,
        zlState::windowH::ID
    };

    void valueChanged(juce::Value &) override;

    void parameterChanged(const juce::String &parameterID, float newValue) override;

    void handleAsyncUpdate() override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
