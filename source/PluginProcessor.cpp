// Copyright (C) 2025 - zsliu98
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
PluginProcessor::PluginProcessor() :
    AudioProcessor(BusesProperties()
                   .withInput("Input", juce::AudioChannelSet::stereo(), true)
                   .withInput("Aux", juce::AudioChannelSet::stereo(), true)
                   .withOutput("Output", juce::AudioChannelSet::stereo(), true)
        ),
    dummy_processor_(),
    parameters_(*this, nullptr,
                juce::Identifier("ZLEqualizerParameters"),
                zlp::getParameterLayout()),
    parameters_NA_(dummy_processor_, nullptr,
                   juce::Identifier("ZLEqualizerNAParameters"),
                   zlstate::getNAParameterLayout()),
    controller_(*this),
    chore_attachment_(*this, parameters_, controller_),
    analyzer_attachment_(*this, parameters_NA_, controller_),
    ext_side_(*parameters_.getRawParameterValue(zlp::PExtSide::kID)),
    bypass_(*parameters_.getRawParameterValue(zlp::PBypass::kID)) {
    for (size_t i = 0; i < zlp::kBandNum; ++i) {
        filter_attachments_[i] = std::make_unique<zlp::FilterAttach>(*this, parameters_, controller_, i);
        filter_dynamic_attachments_[i] = std::make_unique<zlp::FilterDynamicAttach>(*this, parameters_, controller_, i);
        filter_side_attachments_[i] = std::make_unique<zlp::FilterSideAttach>(*this, parameters_, controller_, i);
    }
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

void PluginProcessor::changeProgramName(int, const juce::String&) {
}

//==============================================================================
void PluginProcessor::prepareToPlay(const double sample_rate, const int samples_per_block) {
    for (size_t i = 0; i < 2; ++i) {
        main_buffer_[i].resize(static_cast<size_t>(samples_per_block));
        main_pointers_[i] = main_buffer_[i].data();
        side_buffer_[i].resize(static_cast<size_t>(samples_per_block));
        side_pointers_[i] = side_buffer_[i].data();
    }
    // determine current channel layout
    const auto* main_bus = getBus(true, 0);
    const auto* aux_bus = getBus(true, 1);
    channel_layout_ = ChannelLayout::kInvalid;
    if (main_bus == nullptr) {
        return;
    }
    if (main_bus->getCurrentLayout() == juce::AudioChannelSet::mono()) {
        if (aux_bus == nullptr || !aux_bus->isEnabled()) {
            channel_layout_ = ChannelLayout::kMain1Aux0;
        } else if (aux_bus->getCurrentLayout() == juce::AudioChannelSet::mono()) {
            channel_layout_ = ChannelLayout::kMain1Aux1;
        } else if (aux_bus->getCurrentLayout() == juce::AudioChannelSet::stereo()) {
            channel_layout_ = ChannelLayout::kMain1Aux2;
        }
    } else if (main_bus->getCurrentLayout() == juce::AudioChannelSet::stereo()) {
        if (aux_bus == nullptr || !aux_bus->isEnabled()) {
            channel_layout_ = ChannelLayout::kMain2Aux0;
        } else if (aux_bus->getCurrentLayout() == juce::AudioChannelSet::mono()) {
            channel_layout_ = ChannelLayout::kMain2Aux1;
        } else if (aux_bus->getCurrentLayout() == juce::AudioChannelSet::stereo()) {
            channel_layout_ = ChannelLayout::kMain2Aux2;
        }
    }
    controller_.prepare(sample_rate, static_cast<size_t>(samples_per_block));
    sample_rate_.store(sample_rate, std::memory_order::relaxed);
}

void PluginProcessor::releaseResources() {
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
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

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    if (bypass_.load(std::memory_order::relaxed) > .5f) {
        processBlockInternal<true>(buffer);
    } else {
        processBlockInternal<false>(buffer);
    }
}

void PluginProcessor::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer&) {
    if (bypass_.load(std::memory_order::relaxed) > .5f) {
        processBlockInternal<true>(buffer);
    } else {
        processBlockInternal<false>(buffer);
    }
}

void PluginProcessor::processBlockBypassed(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    processBlockInternal<true>(buffer);
}

void PluginProcessor::processBlockBypassed(juce::AudioBuffer<double>& buffer, juce::MidiBuffer&) {
    processBlockInternal<true>(buffer);
}

template <bool bypass>
void PluginProcessor::processBlockInternal(juce::AudioBuffer<float>& buffer) {
    juce::ScopedNoDenormals no_denormals;
    if (buffer.getNumSamples() == 0) {
        return; // ignore empty blocks
    }
    const auto c_ext_side = ext_side_.load(std::memory_order::relaxed) > .5f;
    const auto num_samples = static_cast<size_t>(buffer.getNumSamples());
    switch (channel_layout_) {
    case kMain1Aux0: {
        zldsp::vector::copy(main_pointers_[0], buffer.getReadPointer(0), num_samples);
        zldsp::vector::copy(main_pointers_[1], main_pointers_[0], num_samples);
        zldsp::vector::copy(side_pointers_[0], main_pointers_[0], num_samples);
        zldsp::vector::copy(side_pointers_[1], side_pointers_[0], num_samples);
        controller_.template process<bypass>(main_pointers_, side_pointers_, num_samples);
        zldsp::vector::copy(buffer.getWritePointer(0), main_pointers_[0], num_samples);
        break;
    }
    case kMain1Aux1: {
        zldsp::vector::copy(main_pointers_[0], buffer.getReadPointer(0), num_samples);
        zldsp::vector::copy(main_pointers_[1], main_pointers_[0], num_samples);
        if (c_ext_side) {
            zldsp::vector::copy(side_pointers_[0], buffer.getReadPointer(1), num_samples);
        } else {
            zldsp::vector::copy(side_pointers_[0], main_pointers_[0], num_samples);
        }
        zldsp::vector::copy(side_pointers_[1], side_pointers_[0], num_samples);
        controller_.template process<bypass>(main_pointers_, side_pointers_, num_samples);
        zldsp::vector::copy(buffer.getWritePointer(0), main_pointers_[0], num_samples);
        break;
    }
    case kMain1Aux2: {
        zldsp::vector::copy(main_pointers_[0], buffer.getReadPointer(0), num_samples);
        zldsp::vector::copy(main_pointers_[1], main_pointers_[0], num_samples);
        if (c_ext_side) {
            zldsp::vector::copy(side_pointers_[0], buffer.getReadPointer(1), num_samples);
            zldsp::vector::copy(side_pointers_[1], buffer.getReadPointer(2), num_samples);
        } else {
            zldsp::vector::copy(side_pointers_[0], main_pointers_[0], num_samples);
            zldsp::vector::copy(side_pointers_[1], main_pointers_[0], num_samples);
        }

        controller_.template process<bypass>(main_pointers_, side_pointers_, num_samples);
        zldsp::vector::copy(buffer.getWritePointer(0), main_pointers_[0], num_samples);
        break;
    }
    case kMain2Aux0: {
        zldsp::vector::copy(main_pointers_[0], buffer.getReadPointer(0), num_samples);
        zldsp::vector::copy(main_pointers_[1], buffer.getReadPointer(1), num_samples);
        zldsp::vector::copy(side_pointers_[0], main_pointers_[0], num_samples);
        zldsp::vector::copy(side_pointers_[1], main_pointers_[1], num_samples);
        controller_.template process<bypass>(main_pointers_, side_pointers_, num_samples);
        zldsp::vector::copy(buffer.getWritePointer(0), main_pointers_[0], num_samples);
        zldsp::vector::copy(buffer.getWritePointer(1), main_pointers_[1], num_samples);
        break;
    }
    case kMain2Aux1: {
        zldsp::vector::copy(main_pointers_[0], buffer.getReadPointer(0), num_samples);
        zldsp::vector::copy(main_pointers_[1], buffer.getReadPointer(1), num_samples);
        if (c_ext_side) {
            zldsp::vector::copy(side_pointers_[0], buffer.getReadPointer(2), num_samples);
            zldsp::vector::copy(side_pointers_[1], buffer.getReadPointer(2), num_samples);
        } else {
            zldsp::vector::copy(side_pointers_[0], main_pointers_[0], num_samples);
            zldsp::vector::copy(side_pointers_[1], main_pointers_[1], num_samples);
        }
        controller_.template process<bypass>(main_pointers_, side_pointers_, num_samples);
        zldsp::vector::copy(buffer.getWritePointer(0), main_pointers_[0], num_samples);
        zldsp::vector::copy(buffer.getWritePointer(1), main_pointers_[1], num_samples);
        break;
    }
    case kMain2Aux2: {
        zldsp::vector::copy(main_pointers_[0], buffer.getReadPointer(0), num_samples);
        zldsp::vector::copy(main_pointers_[1], buffer.getReadPointer(1), num_samples);
        if (c_ext_side) {
            zldsp::vector::copy(side_pointers_[0], buffer.getReadPointer(2), num_samples);
            zldsp::vector::copy(side_pointers_[1], buffer.getReadPointer(3), num_samples);
        } else {
            zldsp::vector::copy(side_pointers_[0], main_pointers_[0], num_samples);
            zldsp::vector::copy(side_pointers_[1], main_pointers_[1], num_samples);
        }
        controller_.template process<bypass>(main_pointers_, side_pointers_, num_samples);
        zldsp::vector::copy(buffer.getWritePointer(0), main_pointers_[0], num_samples);
        zldsp::vector::copy(buffer.getWritePointer(1), main_pointers_[1], num_samples);
        break;
    }
    case kInvalid: {
        return;
    }
    }
}

template <bool bypass>
void PluginProcessor::processBlockInternal(juce::AudioBuffer<double>& buffer) {
    juce::ScopedNoDenormals no_denormals;
    if (buffer.getNumSamples() == 0) {
        return; // ignore empty blocks
    }
    const auto c_ext_side = ext_side_.load(std::memory_order::relaxed) > .5f;
    const auto num_samples = static_cast<size_t>(buffer.getNumSamples());
    switch (channel_layout_) {
    case kMain1Aux0: {
        main_pointers_[0] = buffer.getWritePointer(0);
        zldsp::vector::copy(main_pointers_[1], main_pointers_[0], num_samples);
        zldsp::vector::copy(side_pointers_[0], main_pointers_[0], num_samples);
        zldsp::vector::copy(side_pointers_[1], side_pointers_[0], num_samples);
        controller_.template process<bypass>(main_pointers_, side_pointers_, num_samples);
        break;
    }
    case kMain1Aux1: {
        main_pointers_[0] = buffer.getWritePointer(0);
        zldsp::vector::copy(main_pointers_[1], main_pointers_[0], num_samples);
        if (c_ext_side) {
            side_pointers_[0] = buffer.getWritePointer(1);
        } else {
            zldsp::vector::copy(side_pointers_[0], main_pointers_[0], num_samples);
        }
        zldsp::vector::copy(side_pointers_[1], side_pointers_[0], num_samples);
        controller_.template process<bypass>(main_pointers_, side_pointers_, num_samples);
        break;
    }
    case kMain1Aux2: {
        main_pointers_[0] = buffer.getWritePointer(0);
        zldsp::vector::copy(main_pointers_[1], main_pointers_[0], num_samples);
        if (c_ext_side) {
            side_pointers_[0] = buffer.getWritePointer(1);
            side_pointers_[1] = buffer.getWritePointer(2);
        } else {
            zldsp::vector::copy(side_pointers_[0], main_pointers_[0], num_samples);
            zldsp::vector::copy(side_pointers_[1], side_pointers_[0], num_samples);
        }
        controller_.template process<bypass>(main_pointers_, side_pointers_, num_samples);
        break;
    }
    case kMain2Aux0: {
        main_pointers_[0] = buffer.getWritePointer(0);
        main_pointers_[1] = buffer.getWritePointer(1);
        zldsp::vector::copy(side_pointers_[0], main_pointers_[0], num_samples);
        zldsp::vector::copy(side_pointers_[1], main_pointers_[1], num_samples);
        controller_.template process<bypass>(main_pointers_, side_pointers_, num_samples);
        break;
    }
    case kMain2Aux1: {
        main_pointers_[0] = buffer.getWritePointer(0);
        main_pointers_[1] = buffer.getWritePointer(1);
        if (c_ext_side) {
            zldsp::vector::copy(side_pointers_[0], buffer.getReadPointer(2), num_samples);
            zldsp::vector::copy(side_pointers_[1], side_pointers_[0], num_samples);
        } else {
            zldsp::vector::copy(side_pointers_[0], main_pointers_[0], num_samples);
            zldsp::vector::copy(side_pointers_[1], main_pointers_[1], num_samples);
        }
        controller_.template process<bypass>(main_pointers_, side_pointers_, num_samples);
        break;
    }
    case kMain2Aux2: {
        main_pointers_[0] = buffer.getWritePointer(0);
        main_pointers_[1] = buffer.getWritePointer(1);
        if (c_ext_side) {
            zldsp::vector::copy(side_pointers_[0], buffer.getReadPointer(2), num_samples);
            zldsp::vector::copy(side_pointers_[1], buffer.getReadPointer(3), num_samples);
        } else {
            zldsp::vector::copy(side_pointers_[0], main_pointers_[0], num_samples);
            zldsp::vector::copy(side_pointers_[1], main_pointers_[1], num_samples);
        }
        controller_.template process<bypass>(main_pointers_, side_pointers_, num_samples);
        break;
    }
    case kInvalid: {
        return;
    }
    }
}

bool PluginProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor* PluginProcessor::createEditor() {
    return new PluginEditor(*this);
    // return new juce::GenericAudioProcessorEditor(*this);
}

void PluginProcessor::getStateInformation(juce::MemoryBlock& dest_data) {
    auto temp_tree = juce::ValueTree("ZLCompressorParaState");
    temp_tree.appendChild(parameters_.copyState(), nullptr);
    temp_tree.appendChild(parameters_NA_.copyState(), nullptr);
    const std::unique_ptr<juce::XmlElement> xml(temp_tree.createXml());
    copyXmlToBinary(*xml, dest_data);
}

void PluginProcessor::setStateInformation(const void* data, const int size_in_bytes) {
    std::unique_ptr<juce::XmlElement> xml_state(getXmlFromBinary(data, size_in_bytes));
    if (xml_state != nullptr && xml_state->hasTagName("ZLCompressorParaState")) {
        const auto temp_tree = juce::ValueTree::fromXml(*xml_state);
        parameters_.replaceState(temp_tree.getChildWithName(parameters_.state.getType()));
        parameters_NA_.replaceState(temp_tree.getChildWithName(parameters_NA_.state.getType()));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE

createPluginFilter() {
    return new PluginProcessor();
}
