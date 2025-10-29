// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <string>
#include <array>
#include <cmath>

namespace zlpanel::freq_note {
    inline constexpr std::array kNoteNames = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    inline constexpr int kNotesInOctave = 12;
    inline constexpr double kA4Frequency = 440.0;
    inline constexpr int kA4SemitoneIndexFromC0 = 57;

    inline std::string getNoteFromFrequency(const double frequency) {
        const double semitones_from_a4 = 12.0 * (std::log(frequency / kA4Frequency) / std::log(2.0));
        const int rounded_semitones_from_a4 = static_cast<int>(std::round(semitones_from_a4));
        const int total_semitones_from_c0 = rounded_semitones_from_a4 + kA4SemitoneIndexFromC0;
        const int octave = static_cast<int>(std::floor(static_cast<double>(total_semitones_from_c0) / kNotesInOctave));
        const int note_index = (total_semitones_from_c0 % kNotesInOctave + kNotesInOctave) % kNotesInOctave;
        return kNoteNames[static_cast<size_t>(note_index)] + std::to_string(octave);
    }

    inline std::optional<double> getFrequencyFromNote(const std::string& note) {
        if (note.empty()) {
            return std::nullopt;
        }
        const std::size_t split_point = note.find_first_of("-0123456789");
        if (split_point == 0 || split_point == std::string::npos) {
            return std::nullopt;
        }
        const std::string note_part = note.substr(0, split_point);
        const std::string octave_part = note.substr(split_point);
        int note_index = -1;
        for (int i = 0; i < static_cast<int>(kNoteNames.size()); ++i) {
            if (kNoteNames[static_cast<size_t>(i)] == note_part) {
                note_index = i;
                break;
            }
        }
        if (note_index == -1) {
            return std::nullopt;
        }
        int octave;
        try {
            octave = std::stoi(octave_part);
        } catch (const std::invalid_argument&) {
            return std::nullopt;
        } catch (const std::out_of_range&) {
            return std::nullopt;
        }
        const int total_semitones_from_c0 = (octave * kNotesInOctave) + note_index;
        const int semitones_from_a4 = total_semitones_from_c0 - kA4SemitoneIndexFromC0;
        return kA4Frequency * std::pow(2.0, static_cast<double>(semitones_from_a4) / 12.0);
    }
}
