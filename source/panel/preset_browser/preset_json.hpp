// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace zlpanel {
    class PresetJson final {
    public:
        static juce::Result write(const juce::File& file, const juce::MemoryBlock& processor_state);

        static juce::Result read(const juce::File& file, juce::MemoryBlock& processor_state);

    private:
        static juce::var valueTreeToJson(const juce::ValueTree& tree);

        static juce::Result jsonToValueTree(const juce::var& value, juce::ValueTree& tree);
    };
}
