// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_PROPERTY_H
#define ZL_PROPERTY_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlState {

    class Property {
    public:
        Property();

        explicit Property(juce::AudioProcessorValueTreeState &apvts);

        void loadAPVTS(juce::AudioProcessorValueTreeState &apvts);

        void saveAPVTS(juce::AudioProcessorValueTreeState &apvts);

    private:
        std::unique_ptr<juce::PropertiesFile> uiFile;
        juce::ReadWriteLock readWriteLock;

        inline auto static const path =
                juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                        .getChildFile("Audio")
                        .getChildFile("Presets")
                        .getChildFile(JucePlugin_Manufacturer)
                        .getChildFile(JucePlugin_Name);
        inline auto static const uiPath =
                path.getChildFile("ui.xml");
    };

}

#endif // ZL_PROPERTY_H
