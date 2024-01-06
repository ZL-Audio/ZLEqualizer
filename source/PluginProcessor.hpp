#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "dsp/dsp.hpp"
#include "state/state.hpp"

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor {
public:
    zlState::DummyProcessor dummyProcessor;
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState parametersNA;
    // juce::AudioProcessorValueTreeState state;

    PluginProcessor();

    ~PluginProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    juce::AudioProcessorEditor *createEditor() override;

    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;

    bool producesMidi() const override;

    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    int getNumPrograms() override;

    int getCurrentProgram() override;

    void setCurrentProgram(int index) override;

    const juce::String getProgramName(int index) override;

    void changeProgramName(int index, const juce::String &newName) override;

    void getStateInformation(juce::MemoryBlock &destData) override;

    void setStateInformation(const void *data, int sizeInBytes) override;

    inline zlDSP::Controller<float>& getController() {return controller;}

private:
    zlDSP::Controller<float> controller;
    zlDSP::FiltersAttach<float> filtersAttach;
    zlDSP::SoloAttach<float> soloAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
