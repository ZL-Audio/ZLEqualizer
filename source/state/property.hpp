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

namespace zlstate {
    class Property {
    public:
        Property();

        explicit Property(juce::AudioProcessorValueTreeState& apvts);

        void loadAPVTS(juce::AudioProcessorValueTreeState& apvts);

        void saveAPVTS(juce::AudioProcessorValueTreeState& apvts);

    private:
        const juce::File kOldPath =
            juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("Audio")
            .getChildFile("Presets")
            .getChildFile(JucePlugin_Manufacturer)
            .getChildFile(JucePlugin_Name);
        const juce::File kOldUIPath = kOldPath.getChildFile("ui.xml");

        const juce::File kPath =
            juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("ZL Audio")
            .getChildFile(JucePlugin_Name);
        const juce::File kUIPath = kPath.getChildFile("ui.xml");

        std::mutex mutex_;

        bool checkCreateDirectory() const;
    };
}
