// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include "../vector/kfr_import.hpp"

namespace zldsp::analyzer {
    template <typename FloatType>
    class FFTCollisionAnalyzer {
    public:
        static void createGradientPs(std::span<FloatType> db0, std::span<FloatType> db1,
                                     std::span<FloatType> ps, std::span<FloatType> final_ps,
                                     const FloatType strength) {
            auto db0_v = kfr::make_univector(db0);
            auto db1_v = kfr::make_univector(db1);
            auto p_v = kfr::make_univector(ps);
            auto final_p_v = kfr::make_univector(final_ps);

            const auto db0_avg = kfr::mean(db0_v);
            const auto db1_avg = kfr::mean(db1_v);

            if (!std::isfinite(db0_avg) || !std::isfinite(db1_avg)
                || db0_avg < static_cast<FloatType>(-120) || db1_avg < static_cast<FloatType>(-120)) {
                final_p_v = final_p_v * static_cast<FloatType>(0.95);
                return;
            }

            const auto threshold = std::min(static_cast<FloatType>(0), strength * (db0_avg + db1_avg));
            const auto scale = static_cast<FloatType>(1) / (static_cast<FloatType>(0.1) - threshold);

            p_v = (kfr::min(db0_v, db1_v) - threshold) * scale;
            FloatType p0 = ps[0];
            FloatType p1 = calculateGM(&ps[0], 3);
            const FloatType p_last = calculateGM(&ps[ps.size() - 3], 3);
            for (size_t i = 0; i < ps.size() - 4; ++i) {
                const auto next = calculateGM(&ps[i], 5);
                ps[i] = p0;
                p0 = p1;
                p1 = next;
            }
            ps[ps.size() - 4] = p0;
            ps[ps.size() - 3] = p1;
            ps[ps.size() - 2] = p_last;
            if (const auto mean_p_v = kfr::mean(p_v); mean_p_v > strength * strength) {
                p_v = p_v * (FloatType(0.1) / mean_p_v);
            }
            p_v = kfr::clamp(p_v, FloatType(0.1), FloatType(1)) - FloatType(0.1);
            final_p_v = kfr::max(final_p_v * static_cast<FloatType>(0.95), p_v);
        }

        static FloatType calculateGM(const FloatType *data, size_t n) {
            FloatType sum{};
            for (size_t k = 0; k < n; ++k) {
                if (data[k] < FloatType(0.01)) {
                    return FloatType(0);
                }
                sum += FloatType(1) / data[k];
            }
            return static_cast<FloatType>(n) / sum;
        }
    };
}
