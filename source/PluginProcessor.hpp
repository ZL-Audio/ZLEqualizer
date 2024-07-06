// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

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
    juce::AudioProcessorValueTreeState state;

    PluginProcessor();

    ~PluginProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    void processBlock(juce::AudioBuffer<double> &, juce::MidiBuffer &) override;

    void processBlockBypassed(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;

    void processBlockBypassed(juce::AudioBuffer<double> &buffer, juce::MidiBuffer &midiMessages) override;

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

    bool supportsDoublePrecisionProcessing() const override { return true; }

    inline zlDSP::Controller<double>& getController() {return controller;}

private:
    zlDSP::Controller<double> controller;
    zlDSP::FiltersAttach<double> filtersAttach;
    zlDSP::SoloAttach<double> soloAttach;
    zlDSP::ChoreAttach<double> choreAttach;
    zlDSP::ResetAttach<double> resetAttach;
    juce::AudioBuffer<double> doubleBuffer;
    std::atomic<bool> isMono{false};
    std::atomic<int> mainInChannelNum{2}, auxInChannelNum{2};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
