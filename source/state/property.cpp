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
    }

    Property::Property(juce::AudioProcessorValueTreeState& apvts) {
        loadAPVTS(apvts);
    }

    void Property::loadAPVTS(juce::AudioProcessorValueTreeState& apvts) {
        std::lock_guard<std::mutex> lock_guard{mutex_};
        if (checkCreateDirectory()) {
            if (const auto xml = juce::XmlDocument::parse(kUIPath); xml) {
                apvts.replaceState(juce::ValueTree::fromXml(*xml));
            }
        }
    }

    void Property::saveAPVTS(juce::AudioProcessorValueTreeState& apvts) {
        std::lock_guard<std::mutex> lock{mutex_};
        if (checkCreateDirectory()) {
            if (const auto xml = apvts.copyState().createXml(); xml) {
                if (!xml->writeTo(kUIPath)) {
                    return;
                }
            }
        }
    }

    bool Property::checkCreateDirectory() const {
        // create directory if not exists
        if (!kPath.isDirectory()) {
            if (!kPath.createDirectory()) {
                return false;
            }
        }
        // check if UI preset exists
        if (kUIPath.existsAsFile()) {
            return true;
        }
        // check if old UI preset exists
        // yes -> copy old UI preset to UI preset
        if (kOldUIPath.existsAsFile()) {
            if (const auto c_res = kOldUIPath.copyFileTo(kUIPath); c_res) {
                if (const auto d_res = kOldUIPath.deleteFile(); d_res) {
                    return true;
                }
            }
        }
        // no -> create a blank UI preset
        const auto res = kUIPath.create();
        return res.wasOk();
    }
}
