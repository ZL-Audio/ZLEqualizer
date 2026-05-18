// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>

#include "../../vector/vector.hpp"

namespace zldsp::analyzer {
    class SpectrumTilter {
    public:
        explicit SpectrumTilter() = default;

        void prepare(const size_t fft_size) {
            tilt_shift_.resize(fft_size / 2 + 1);
        }

        void setTiltSlope(const double sample_rate, const double slope_per_oct) {
            const auto delta = sample_rate * 0.5 / static_cast<double>(tilt_shift_.size() - 1);
            for (size_t i = 1; i < tilt_shift_.size(); ++i) {
                const auto freq = static_cast<double>(i) * delta;
                tilt_shift_[i] = static_cast<float>(std::log2(freq / 1000.0) * slope_per_oct);
            }
            tilt_shift_[0] = tilt_shift_[1];
        }

        void tilt(std::span<float> spectrum_db) {
            vector::add(spectrum_db.data(), tilt_shift_.data(), spectrum_db.size());
        }

    private:
        vector::aligned_vector<float> tilt_shift_{};
    };
}
