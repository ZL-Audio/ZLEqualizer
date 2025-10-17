// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

namespace zlpanel::freq_helper {
    inline double getSliderMax(const double sample_rate) {
        const double ratio441 = sample_rate / 44100.0;
        if (std::fabs(ratio441 - std::round(ratio441)) < 1e-3) {
            return std::round(sample_rate * (200.0 / 441.0));
        } else {
            return std::floor(sample_rate * (200.0 / 441.0));
        }
    }

    inline double getFFTMax(const double sample_rate) {
        if (sample_rate > 352000.0) {
            return 176000.0;
        } else if (sample_rate > 176000.0) {
            return 88000.0;
        } else if (sample_rate > 88000.0) {
            return 44000.0;
        } else {
            return 22000.0;
        }
    }
}
