// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

//==============================================================================
PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withInput("Aux", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)
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
      property(state),
      controller(*this, zlState::ffTOrder::orders[
                     static_cast<size_t>(state.getRawParameterValue(zlState::ffTOrder::ID)->load())]),
      filtersAttach(*this, parameters, parametersNA, controller),
      soloAttach(*this, parameters, controller),
      choreAttach(*this, parameters, parametersNA, controller) {
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
void PluginProcessor::prepareToPlay(const double sampleRate, const int samplesPerBlock) {
    // prepare to play
    const juce::dsp::ProcessSpec spec{
        sampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        2
    };
    doubleBuffer.setSize(4, samplesPerBlock);
    doubleBuffer.clear();
    controller.prepare(spec);
    // determine current channel layout
    const auto *mainBus = getBus(true, 0);
    const auto *auxBus = getBus(true, 1);
    channelLayout = ChannelLayout::invalid;
    if (mainBus == nullptr) {
        return;
    }
    if (mainBus->getCurrentLayout() == juce::AudioChannelSet::mono()) {
        if (auxBus == nullptr || !auxBus->isEnabled()) {
            channelLayout = ChannelLayout::main1aux0;
        } else if (auxBus->getCurrentLayout() == juce::AudioChannelSet::mono()) {
            channelLayout = ChannelLayout::main1aux1;
        } else if (auxBus->getCurrentLayout() == juce::AudioChannelSet::stereo()) {
            channelLayout = ChannelLayout::main1aux2;
        }
    } else if (mainBus->getCurrentLayout() == juce::AudioChannelSet::stereo()) {
        if (auxBus == nullptr || !auxBus->isEnabled()) {
            channelLayout = ChannelLayout::main2aux0;
        } else if (auxBus->getCurrentLayout() == juce::AudioChannelSet::mono()) {
            channelLayout = ChannelLayout::main2aux1;
        } else if (auxBus->getCurrentLayout() == juce::AudioChannelSet::stereo()) {
            channelLayout = ChannelLayout::main2aux2;
        }
    }
}

void PluginProcessor::releaseResources() {
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo() &&
        layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo() &&
        (layouts.getChannelSet(true, 1).isDisabled() ||
         layouts.getChannelSet(true, 1) == juce::AudioChannelSet::mono() ||
         layouts.getChannelSet(true, 1) == juce::AudioChannelSet::stereo())) {
        return true;
    }
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono() &&
        (layouts.getChannelSet(true, 1).isDisabled() ||
         layouts.getChannelSet(true, 1) == juce::AudioChannelSet::mono() ||
         layouts.getChannelSet(true, 1) == juce::AudioChannelSet::stereo())) {
        return true;
    }
    return false;
}

void PluginProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                   juce::MidiBuffer &midiMessages) {
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    doubleBuffer.setSize(4, buffer.getNumSamples(), false, false, true);
    switch (channelLayout) {
        case ChannelLayout::main1aux0: {
            doubleBufferCopyFrom(0, buffer, 0);
            doubleBufferCopyFrom(1, buffer, 0);
            doubleBufferCopyFrom(2, buffer, 0);
            doubleBufferCopyFrom(3, buffer, 0);
            controller.process(doubleBuffer);
            doubleBufferCopyTo(0, buffer, 0);
            break;
        }
        case ChannelLayout::main1aux1: {
            doubleBufferCopyFrom(0, buffer, 0);
            doubleBufferCopyFrom(1, buffer, 0);
            doubleBufferCopyFrom(2, buffer, 1);
            doubleBufferCopyFrom(3, buffer, 1);
            controller.process(doubleBuffer);
            doubleBufferCopyTo(0, buffer, 0);
            break;
        }
        case ChannelLayout::main1aux2: {
            doubleBufferCopyFrom(0, buffer, 0);
            doubleBufferCopyFrom(1, buffer, 0);
            doubleBufferCopyFrom(2, buffer, 1);
            doubleBufferCopyFrom(3, buffer, 2);
            controller.process(doubleBuffer);
            doubleBufferCopyTo(0, buffer, 0);
            break;
        }
        case ChannelLayout::main2aux0: {
            doubleBufferCopyFrom(0, buffer, 0);
            doubleBufferCopyFrom(1, buffer, 1);
            doubleBufferCopyFrom(2, buffer, 0);
            doubleBufferCopyFrom(3, buffer, 1);
            controller.process(doubleBuffer);
            doubleBufferCopyTo(0, buffer, 0);
            doubleBufferCopyTo(1, buffer, 1);
            break;
        }
        case ChannelLayout::main2aux1: {
            doubleBufferCopyFrom(0, buffer, 0);
            doubleBufferCopyFrom(1, buffer, 1);
            doubleBufferCopyFrom(2, buffer, 2);
            doubleBufferCopyFrom(3, buffer, 2);
            controller.process(doubleBuffer);
            doubleBufferCopyTo(0, buffer, 0);
            doubleBufferCopyTo(1, buffer, 1);
            break;
        }
        case ChannelLayout::main2aux2: {
            doubleBufferCopyFrom(0, buffer, 0);
            doubleBufferCopyFrom(1, buffer, 1);
            doubleBufferCopyFrom(2, buffer, 2);
            doubleBufferCopyFrom(3, buffer, 3);
            controller.process(doubleBuffer);
            doubleBufferCopyTo(0, buffer, 0);
            doubleBufferCopyTo(1, buffer, 1);
            break;
        }
        case ChannelLayout::invalid: {
        }
    }
}

void PluginProcessor::processBlock(juce::AudioBuffer<double> &buffer, juce::MidiBuffer &midiMessages) {
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    doubleBuffer.setSize(4, buffer.getNumSamples(), false, false, true);
    switch (channelLayout) {
        case ChannelLayout::main1aux0: {
            doubleBufferCopyFrom(0, buffer, 0);
            doubleBufferCopyFrom(1, buffer, 0);
            doubleBufferCopyFrom(2, buffer, 0);
            doubleBufferCopyFrom(3, buffer, 0);
            controller.process(doubleBuffer);
            doubleBufferCopyTo(0, buffer, 0);
            break;
        }
        case ChannelLayout::main1aux1: {
            doubleBufferCopyFrom(0, buffer, 0);
            doubleBufferCopyFrom(1, buffer, 0);
            doubleBufferCopyFrom(2, buffer, 1);
            doubleBufferCopyFrom(3, buffer, 1);
            controller.process(doubleBuffer);
            doubleBufferCopyTo(0, buffer, 0);
            break;
        }
        case ChannelLayout::main1aux2: {
            doubleBufferCopyFrom(0, buffer, 0);
            doubleBufferCopyFrom(1, buffer, 0);
            doubleBufferCopyFrom(2, buffer, 1);
            doubleBufferCopyFrom(3, buffer, 2);
            controller.process(doubleBuffer);
            doubleBufferCopyTo(0, buffer, 0);
            break;
        }
        case ChannelLayout::main2aux0: {
            doubleBufferCopyFrom(0, buffer, 0);
            doubleBufferCopyFrom(1, buffer, 1);
            doubleBufferCopyFrom(2, buffer, 0);
            doubleBufferCopyFrom(3, buffer, 1);
            controller.process(doubleBuffer);
            doubleBufferCopyTo(0, buffer, 0);
            doubleBufferCopyTo(1, buffer, 1);
            break;
        }
        case ChannelLayout::main2aux1: {
            doubleBufferCopyFrom(0, buffer, 0);
            doubleBufferCopyFrom(1, buffer, 1);
            doubleBufferCopyFrom(2, buffer, 2);
            doubleBufferCopyFrom(3, buffer, 2);
            controller.process(doubleBuffer);
            doubleBufferCopyTo(0, buffer, 0);
            doubleBufferCopyTo(1, buffer, 1);
            break;
        }
        case ChannelLayout::main2aux2: {
            controller.process(buffer);
            break;
        }
        case ChannelLayout::invalid: {
        }
    }
}

void PluginProcessor::processBlockBypassed(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) {
    juce::ignoreUnused(buffer, midiMessages);
}

void PluginProcessor::processBlockBypassed(juce::AudioBuffer<double> &buffer, juce::MidiBuffer &midiMessages) {
    juce::ignoreUnused(buffer, midiMessages);
}

bool PluginProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor *PluginProcessor::createEditor() {
    return new PluginEditor(*this);
}

void PluginProcessor::getStateInformation(juce::MemoryBlock &destData) {
    auto tempTree = juce::ValueTree("ZLEqualizerParaState");
    tempTree.appendChild(parameters.copyState(), nullptr);
    tempTree.appendChild(parametersNA.copyState(), nullptr);
    const std::unique_ptr<juce::XmlElement> xml(tempTree.createXml());
    copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName("ZLEqualizerParaState")) {
        const auto tempTree = juce::ValueTree::fromXml(*xmlState);
        parameters.replaceState(tempTree.getChildWithName(parameters.state.getType()));
        parametersNA.replaceState(tempTree.getChildWithName(parametersNA.state.getType()));
    }
}

void PluginProcessor::doubleBufferCopyFrom(const int destChan,
                                           const juce::AudioBuffer<float> &buffer, const int srcChan) {
    auto *dest = doubleBuffer.getWritePointer(destChan);
    auto *src = buffer.getReadPointer(srcChan);
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        dest[i] = static_cast<double>(src[i]);
    }
}

void PluginProcessor::doubleBufferCopyFrom(const int destChan,
                                           const juce::AudioBuffer<double> &buffer, const int srcChan) {
    juce::FloatVectorOperations::copy(doubleBuffer.getWritePointer(destChan),
                                      buffer.getReadPointer(srcChan),
                                      buffer.getNumSamples());
}

void PluginProcessor::doubleBufferCopyTo(const int srcChan,
                                         juce::AudioBuffer<float> &buffer, int const destChan) const {
    auto *src = doubleBuffer.getReadPointer(srcChan);
    auto *dest = buffer.getWritePointer(destChan);
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        dest[i] = static_cast<float>(src[i]);
    }
}

void PluginProcessor::doubleBufferCopyTo(const int srcChan,
                                         juce::AudioBuffer<double> &buffer, int const destChan) const {
    juce::FloatVectorOperations::copy(buffer.getWritePointer(destChan),
                                      doubleBuffer.getReadPointer(srcChan),
                                      buffer.getNumSamples());
}

juce::AudioProcessor *JUCE_CALLTYPE

createPluginFilter() {
    return new PluginProcessor();
}
