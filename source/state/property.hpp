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
#include <juce_dsp/juce_dsp.h>

namespace zlstate {
    class Property {
    public:
        Property();

        explicit Property(juce::AudioProcessorValueTreeState &apvts);

        void loadAPVTS(juce::AudioProcessorValueTreeState &apvts);

        void saveAPVTS(juce::AudioProcessorValueTreeState &apvts);

    private:
        std::unique_ptr<juce::PropertiesFile> ui_file_;
        juce::ReadWriteLock read_write_lock_;

        inline auto static const kPath =
                juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("Audio")
                .getChildFile("Presets")
                .getChildFile(JucePlugin_Manufacturer)
                .getChildFile(JucePlugin_Name);
        inline auto static const kUIPath =
                kPath.getChildFile("ui.xml");
    };
}
