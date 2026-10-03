// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <cmath>

#include "panel_constants.hpp"

namespace zlpanel::piano_roll_helper {
    inline constexpr double kMinFrequency = 10.0;

    inline int getHeight(const float font_size) {
        return static_cast<int>(std::round(font_size * 1.75f));
    }

    inline double frequencyToNote(const double frequency) {
        return 69.0 + 12.0 * std::log2(frequency / 440.0);
    }

    inline double noteToFrequency(const double note) {
        return 440.0 * std::exp2((note - 69.0) / 12.0);
    }

    inline bool isBlackKey(const int note) {
        const auto pitch_class = (note % 12 + 12) % 12;
        return pitch_class == 1 || pitch_class == 3 || pitch_class == 6
            || pitch_class == 8 || pitch_class == 10;
    }

    inline float frequencyToX(const double frequency, const float width, const double fft_max) {
        return width * kFFTSizeOverWidth * static_cast<float>(std::log(frequency / 10.0)
            / std::log(fft_max / 10.0));
    }

    inline double xToFrequency(const float x, const float width, const double fft_max) {
        return 10.0 * std::exp(static_cast<double>(x / (width * kFFTSizeOverWidth))
            * std::log(fft_max / 10.0));
    }

    inline double snapFrequency(const double frequency, const double min_frequency, const double max_frequency) {
        const auto first_note = std::ceil(frequencyToNote(min_frequency));
        const auto last_note = std::floor(frequencyToNote(max_frequency));
        if (last_note < first_note) {
            return min_frequency;
        }
        return noteToFrequency(std::clamp(std::round(frequencyToNote(frequency)), first_note, last_note));
    }
}
