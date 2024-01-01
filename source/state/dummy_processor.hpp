// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_DUMMY_PROCESSOR_H
#define ZL_DUMMY_PROCESSOR_H

#include <juce_audio_processors/juce_audio_processors.h>

namespace zlState {
    class DummyProcessor : public juce::AudioProcessor {
    public:
        DummyProcessor() = default;

        ~DummyProcessor() override = default;

        void prepareToPlay(double, int) override {
        }

        void releaseResources() override {
        }

        bool isBusesLayoutSupported(const BusesLayout &) const override { return true; }

        void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override {
        }

        juce::AudioProcessorEditor *createEditor() override { return nullptr; }

        bool hasEditor() const override { return false; }

        const juce::String getName() const override { return {}; }

        bool acceptsMidi() const override { return false; }

        bool producesMidi() const override { return false; }

        bool isMidiEffect() const override { return false; }

        double getTailLengthSeconds() const override { return 0; }

        int getNumPrograms() override { return 1; }

        int getCurrentProgram() override { return 0; }

        void setCurrentProgram(int) override {
        }

        const juce::String getProgramName(int) override { return {}; }

        void changeProgramName(int, const juce::String &) override {
        }

        void getStateInformation(juce::MemoryBlock &) override {
        }

        void setStateInformation(const void *, int) override {
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DummyProcessor)
    };
}
#endif //ZL_DUMMY_PROCESSOR_H
