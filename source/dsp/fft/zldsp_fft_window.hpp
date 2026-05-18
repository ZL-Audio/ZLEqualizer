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
#include <cmath>
#include "zldsp_fft/src/common/zldsp_fft_common_math.hpp"

namespace zldsp::fft {
    /**
     * create periodic Hanning window
     * @tparam F
     * @param window
     */
    template <typename F>
    void createPeriodicHanning(std::span<F> window) {
        const double two_over_n = 2.0 / static_cast<double>(window.size());
        for (size_t i = 0; i < window.size(); ++i) {
            window[i] = static_cast<F>(0.5 * (1.0 - common::math::cospi(static_cast<double>(i) * two_over_n)));
        }
    }

    /**
     * create periodic Hanning window
     * @tparam F
     * @param window
     * @param scale
     */
    template <typename F>
    void createPeriodicHanning(std::span<F> window, const F scale) {
        const double two_over_n = 2.0 / static_cast<double>(window.size());
        for (size_t i = 0; i < window.size(); ++i) {
            window[i] = scale * static_cast<F>(0.5 * (1.0 - common::math::cospi(static_cast<double>(i) * two_over_n)));
        }
    }
}
