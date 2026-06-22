// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>

namespace zlpanel::freq_helper {
    inline double getSliderMax(const double sample_rate) {
        if (sample_rate < 50000.0) {
            return 30000.0;
        } else {
            return 0.49964 * sample_rate;
        }
    }

    inline double getFFTMax(const double sample_rate) {
        if (sample_rate < 50000.0) {
            return 30000.0;
        } else {
            return 0.5 * sample_rate;
        }
    }
}
