// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "dsp/dsp.hpp"
#include "state/state.hpp"

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor {
public:
    zlstate::DummyProcessor dummy_processor_;
    juce::AudioProcessorValueTreeState parameters_;
    juce::AudioProcessorValueTreeState parameters_NA_;
    juce::AudioProcessorValueTreeState state_;
    zlstate::Property property_;

    PluginProcessor();

    ~PluginProcessor() override;

    void prepareToPlay(double sample_rate, int samples_per_block) override;

    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    void processBlock(juce::AudioBuffer<double> &, juce::MidiBuffer &) override;

    void processBlockBypassed(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &) override;

    void processBlockBypassed(juce::AudioBuffer<double> &buffer, juce::MidiBuffer &) override;

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

    void changeProgramName(int, const juce::String &) override;

    void getStateInformation(juce::MemoryBlock &dest_data) override;

    void setStateInformation(const void *data, int size_in_bytes) override;

    bool supportsDoublePrecisionProcessing() const override { return true; }

    inline zlp::Controller<double> &getController() { return controller_; }

    inline zlp::FiltersAttach<double> &getFiltersAttach() { return filters_attach_; }

private:
    zlp::Controller<double> controller_;
    zlp::FiltersAttach<double> filters_attach_;
    zlp::SoloAttach<double> solo_attach_;
    zlp::ChoreAttach<double> chore_attach_;
    juce::AudioBuffer<double> double_buffer_;

    enum ChannelLayout {
        kMain1Aux0, kMain1Aux1, kMain1Aux2,
        kMain2Aux0, kMain2Aux1, kMain2Aux2,
        kInvalid
    };
    ChannelLayout channel_layout_{kInvalid};

    void doubleBufferCopyFrom(int dest_chan, const juce::AudioBuffer<float> &buffer, int src_chan);

    void doubleBufferCopyTo(int src_chan, juce::AudioBuffer<float> &buffer, int dest_chan) const;

    void doubleBufferCopyFrom(int dest_chan, const juce::AudioBuffer<double> &buffer, int src_chan);

    void doubleBufferCopyTo(int src_chan, juce::AudioBuffer<double> &buffer, int dest_chan) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
