// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <functional>
#include <span>
#include <vector>

#include "../../dsp/filter/helpers.hpp"

namespace zlchore::eq_match {
    using Bands = std::vector<zldsp::filter::FilterParameters>;

    struct FitResult {
        // index n holds the fit for n bands
        std::vector<Bands> by_band_count{Bands{}};
        size_t suggested_num_bands = 0;
    };

    /**
     * fit the target correction in dB at frequencies in Hz
     * @return completed fits and the suggested band count, even on cancellation
     */
    FitResult fit(double sample_rate, std::span<const float> freqs, std::span<const float> target_db,
                  size_t max_bands, const std::function<bool()>& should_stop);
}
