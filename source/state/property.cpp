// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version. ZLEComp is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#include "property.hpp"

namespace zlState {

    Property::Property() {
        if (!path.isDirectory()) {
            path.createDirectory();
        }
        uiFile = std::make_unique<juce::PropertiesFile>(
                uiPath, juce::PropertiesFile::Options());
    }

    Property::Property(juce::AudioProcessorValueTreeState &apvts) {
        if (!path.isDirectory()) {
            path.createDirectory();
        }
        uiFile = std::make_unique<juce::PropertiesFile>(
                uiPath, juce::PropertiesFile::Options());
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
        auto file = uiFile->getFile();
        if (auto xml = apvts.copyState().createXml()) {
            xml->writeTo(file);
        }
    }
} // namespace zlstate