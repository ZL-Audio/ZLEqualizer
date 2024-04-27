// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

//==============================================================================
PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)
          .withInput("Aux", juce::AudioChannelSet::stereo(), true)
      ), dummyProcessor(),
      parameters(*this, nullptr,
                 juce::Identifier("ZLEqualizerParameters"),
                 zlDSP::getParameterLayout()),
      parametersNA(dummyProcessor, nullptr,
                   juce::Identifier("ZLEqualizerParametersNA"),
                   zlState::getNAParameterLayout()),
      state(dummyProcessor, nullptr,
            juce::Identifier("ZLEqualizerState"),
            zlState::getStateParameterLayout()),
      controller(*this),
      filtersAttach(*this, parameters, parametersNA, controller),
      soloAttach(*this, parameters, controller),
      choreAttach(*this, parameters, parametersNA, controller),
      resetAttach(*this, parameters, parametersNA, controller) {
}

PluginProcessor::~PluginProcessor() = default;

//==============================================================================
const juce::String PluginProcessor::getName() const {
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int PluginProcessor::getNumPrograms() {
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram() {
    return 0;
}

void PluginProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String PluginProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void PluginProcessor::changeProgramName(int index, const juce::String &newName) {
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    // Use this method as the place to do any pre-playback
    // initialisation that you need.
    const auto channels = static_cast<juce::uint32>(juce::jmin(getMainBusNumInputChannels(),
                                                               getMainBusNumOutputChannels()));
    isMono.store(channels == 1);
    const juce::dsp::ProcessSpec spec{
        sampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        2
    };
    doubleBuffer.setSize(4, samplesPerBlock);
    controller.prepare(spec);
}

void PluginProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    controller.reset();
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()) {
        return false;
    }
    if (layouts.getChannelSet(true, 0) != layouts.getChannelSet(true, 1)) {
        return false;
    }
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) {
        return false;
    }
    return true;
}

void PluginProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                   juce::MidiBuffer &midiMessages) {
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    if (isMono.load()) {
        doubleBuffer.setSize(4, buffer.getNumSamples(), false, false, true);
        for (int chan = 0; chan < 4; ++chan) {
            auto *dest = doubleBuffer.getWritePointer(chan);
            auto *src = buffer.getReadPointer(chan / 2);
            for (int i = 0; i < doubleBuffer.getNumSamples(); ++i) {
                dest[i] = static_cast<double>(src[i]);
            }
        }
        controller.process(doubleBuffer); {
            auto *dest = buffer.getWritePointer(0);
            auto *src = doubleBuffer.getReadPointer(0);
            for (int i = 0; i < doubleBuffer.getNumSamples(); ++i) {
                dest[i] = static_cast<float>(src[i]);
            }
        }
    } else {
        doubleBuffer.makeCopyOf(buffer, true);
        controller.process(doubleBuffer);
        for (int chan = 0; chan < 2; ++chan) {
            auto *dest = buffer.getWritePointer(chan);
            auto *src = doubleBuffer.getReadPointer(chan);
            for (int i = 0; i < doubleBuffer.getNumSamples(); ++i) {
                dest[i] = static_cast<float>(src[i]);
            }
        }
    }
}

void PluginProcessor::processBlock(juce::AudioBuffer<double> &buffer, juce::MidiBuffer &midiMessages) {
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    if (isMono.load()) {
        for (int chan = 0; chan < 4; ++chan) {
            auto *dest = doubleBuffer.getWritePointer(chan);
            auto *src = buffer.getReadPointer(chan / 2);
            for (int i = 0; i < doubleBuffer.getNumSamples(); ++i) {
                dest[i] = src[i];
            }
        }
        controller.process(doubleBuffer); {
            auto *dest = buffer.getWritePointer(0);
            auto *src = doubleBuffer.getReadPointer(0);
            for (int i = 0; i < doubleBuffer.getNumSamples(); ++i) {
                dest[i] = src[i];
            }
        }
    } else {
        controller.process(buffer);
    }
}

//==============================================================================
bool PluginProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor *PluginProcessor::createEditor() {
    return new PluginEditor(*this);
    // return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void PluginProcessor::getStateInformation(juce::MemoryBlock &destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto tempTree = juce::ValueTree("ZLEqualizerParaState");
    tempTree.appendChild(parameters.copyState(), nullptr);
    tempTree.appendChild(parametersNA.copyState(), nullptr);
    const std::unique_ptr<juce::XmlElement> xml(tempTree.createXml());
    copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName("ZLEqualizerParaState")) {
        auto tempTree = juce::ValueTree::fromXml(*xmlState);
        filtersAttach.enableDynamicONUpdateOthers(false);
        parameters.replaceState(tempTree.getChildWithName(parameters.state.getType()));
        parametersNA.replaceState(tempTree.getChildWithName(parametersNA.state.getType()));
        filtersAttach.enableDynamicONUpdateOthers(true);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE

createPluginFilter() {
    return new PluginProcessor();
}
