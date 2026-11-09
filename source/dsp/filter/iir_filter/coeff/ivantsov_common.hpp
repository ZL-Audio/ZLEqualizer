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
#include <numbers>

namespace zldsp::filter::ivantsov {
    inline constexpr double kPi = std::numbers::pi;
    inline constexpr double kSigma = 2.00143;
    inline constexpr double kSigmaSq = kSigma * kSigma;

    inline double getWrappingW2(const double w0) {
        if (w0 <= 0.4996419767299294) {
            const auto t = std::tan(kPi * w0);
            return kSigmaSq + kPi * kPi / (t * t);
        }
        if (w0 >= 0.5919981009) {
            return 1.0 / (w0 * w0);
        }
        const auto t = (w0 - 0.4996419767299294) * 10.8276522968691715;
        return t * (t * (1.4079737347730696 * t - 2.5538880428234330)
                    - 0.0064417990487326) + 4.0057345308722958;
    }

    struct ShelfContext {
        double w2, gain_sqrt;
    };

    struct Core {
        double v, d, pi_sq_term;
    };

    inline Core getCore(const double w2, const double zeta2) {
        const auto z_term = 2.0 * zeta2 - 1.0;
        const auto kappa = w2 * z_term + kSigmaSq;
        const auto v = std::sqrt(std::abs(w2 * w2 + 2.0 * kSigmaSq * w2 * z_term + kSigmaSq * kSigmaSq));
        const auto term = kPi * std::sqrt(std::abs(2.0 * (v + kappa)));
        return {v, kPi * kPi + term + v, term};
    }
}
