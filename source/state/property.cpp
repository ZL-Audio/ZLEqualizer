// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "property.hpp"

namespace zlstate {
    Property::Property() {
        if (!kPath.isDirectory()) {
            if (!kPath.createDirectory()) return;
        }
        ui_file_ = std::make_unique<juce::PropertiesFile>(kUIPath, juce::PropertiesFile::Options());
    }

    Property::Property(juce::AudioProcessorValueTreeState &apvts) {
        if (!kPath.isDirectory()) {
            if (!kPath.createDirectory()) return;
        }
        ui_file_ = std::make_unique<juce::PropertiesFile>(kUIPath, juce::PropertiesFile::Options());
        loadAPVTS(apvts);
    }

    void Property::loadAPVTS(juce::AudioProcessorValueTreeState &apvts) {
        const juce::ScopedReadLock scoped_lock(read_write_lock_);
        const auto file = ui_file_->getFile();
        if (const auto xml = juce::XmlDocument::parse(file)) {
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
        }
    }

    void Property::saveAPVTS(juce::AudioProcessorValueTreeState &apvts) {
        const juce::ScopedWriteLock scoped_lock(read_write_lock_);
        const auto file = ui_file_->getFile();
        if (const auto xml = apvts.copyState().createXml()) {
            if (!xml->writeTo(file)) return;
        }
    }
} // namespace zlstate
