// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "property.hpp"

namespace zlState {
    Property::Property() {
        if (!path.isDirectory()) {
            path.createDirectory();
        }
        uiFile = std::make_unique<juce::PropertiesFile>(uiPath, juce::PropertiesFile::Options());
    }

    Property::Property(juce::AudioProcessorValueTreeState &apvts) {
        if (!path.isDirectory()) {
            path.createDirectory();
        }
        uiFile = std::make_unique<juce::PropertiesFile>(uiPath, juce::PropertiesFile::Options());
        loadAPVTS(apvts);
    }

    void Property::loadAPVTS(juce::AudioProcessorValueTreeState &apvts) {
        const juce::ScopedReadLock myScopedLock(readWriteLock);
        auto file = uiFile->getFile();
        if (auto xml = juce::XmlDocument::parse(file)) {
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
        }
    }

    void Property::saveAPVTS(juce::AudioProcessorValueTreeState &apvts) {
        const juce::ScopedWriteLock myScopedLock(readWriteLock);
        const auto file = uiFile->getFile();
        if (const auto xml = apvts.copyState().createXml()) {
            xml->writeTo(file);
        }
    }
} // namespace zlstate