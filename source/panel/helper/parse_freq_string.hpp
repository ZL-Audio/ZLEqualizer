// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

inline std::optional<double> parseFreqPitchString(juce::String s) {
    static constexpr std::array<std::string, 12> kPitchLookUp{
        "A", "A#", "B", "C",
        "C#", "D", "D#", "E",
        "F", "F#", "G", "G#"
    };
    for (size_t i = 0; i < kPitchLookUp.size(); ++i) {
        if (s.startsWithIgnoreCase(kPitchLookUp[11 - i])) {
            auto shift = static_cast<double>(11 - i) / 12.0;
            s = s.substring(static_cast<int>(kPitchLookUp[11 - i].length()));
            shift += s.getDoubleValue();
            return 27.5 * std::pow(2.0, shift);
        }
    }
    return std::nullopt;
}

inline double parseFreqValueString(const juce::String &s) {
    const auto k = (s.contains("k") || s.contains("K")) ? 1000.0 : 1.0;
    return s.getDoubleValue() * k;
}
