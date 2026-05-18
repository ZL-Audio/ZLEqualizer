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
    template <typename FloatType>
    class SpectrumCollision {
    public:
        static void createGradientPs(std::span<FloatType> db0, std::span<FloatType> db1,
                                     std::span<FloatType> ps, std::span<FloatType> final_ps,
                                     const FloatType strength) {
            namespace hn = hwy::HWY_NAMESPACE;

            static constexpr hn::ScalableTag<FloatType> d;
            static constexpr size_t lanes = hn::MaxLanes(d);

            std::array<FloatType, 2> db_avgs{};
            db_avgs[0] = calculateSoftmaxAvg(db0, FloatType(0.1));
            db_avgs[1] = calculateSoftmaxAvg(db1, FloatType(0.1));

            if (!std::isfinite(db_avgs[0]) || !std::isfinite(db_avgs[1])
                || db_avgs[0] < static_cast<FloatType>(-120) || db_avgs[1] < static_cast<FloatType>(-120)) {
                size_t i = 0;
                auto v_decay = hn::Set(d, static_cast<FloatType>(0.95));
                for (; i + lanes <= ps.size(); i += lanes) {
                    auto fp = hn::LoadU(d, final_ps.data() + i);
                    hn::StoreU(hn::Mul(fp, v_decay), d, final_ps.data() + i);
                }
                for (; i < ps.size(); ++i) {
                    final_ps[i] *= static_cast<FloatType>(0.95);
                }
                return;
            }

            const auto db_avg = FloatType(2) * calculateSoftmaxAvg(db_avgs, FloatType(0.1));
            const auto threshold = std::min(static_cast<FloatType>(0), strength * db_avg);
            const auto scale = static_cast<FloatType>(1) / (static_cast<FloatType>(0.1) - threshold);

            const auto v_thresh = hn::Set(d, threshold);
            const auto v_scale = hn::Set(d, scale);
            auto v_sum = hn::Zero(d);
            size_t i = 0;
            for (; i + lanes <= ps.size(); i += lanes) {
                auto d0 = hn::LoadU(d, db0.data() + i);
                auto d1 = hn::LoadU(d, db1.data() + i);
                auto min_val = hn::Min(d0, d1);
                auto p = hn::Mul(hn::Sub(min_val, v_thresh), v_scale);
                hn::StoreU(p, d, ps.data() + i);
                v_sum = hn::Add(v_sum, p);
            }

            FloatType sum_p = hn::ReduceSum(d, v_sum);
            for (; i < ps.size(); ++i) {
                ps[i] = (std::min(db0[i], db1[i]) - threshold) * scale;
                sum_p += ps[i];
            }

            const auto mean_p_v = sum_p / static_cast<FloatType>(ps.size());
            const auto p_mult = (mean_p_v > strength * strength) ? (FloatType(0.1) / mean_p_v) : FloatType(1.0);

            const auto v_p_mult = hn::Set(d, p_mult);
            const auto v_clamp_low = hn::Set(d, FloatType(0.1));
            const auto v_clamp_high = hn::Set(d, FloatType(1.0));
            const auto v_decay = hn::Set(d, FloatType(0.95));

            i = 0;
            for (; i + lanes <= ps.size(); i += lanes) {
                auto p = hn::LoadU(d, ps.data() + i);

                p = hn::Mul(p, v_p_mult);
                p = hn::Clamp(p, v_clamp_low, v_clamp_high);
                p = hn::Sub(p, v_clamp_low);
                hn::StoreU(p, d, ps.data() + i);

                auto fp = hn::LoadU(d, final_ps.data() + i);
                fp = hn::Max(hn::Mul(fp, v_decay), p);
                hn::StoreU(fp, d, final_ps.data() + i);
            }
            for (; i < ps.size(); ++i) {
                ps[i] *= p_mult;
                ps[i] = std::clamp(ps[i], FloatType(0.1), FloatType(1.0)) - FloatType(0.1);
                final_ps[i] = std::max(final_ps[i] * FloatType(0.95), ps[i]);
            }
        }

    private:
        static FloatType calculateSoftmaxAvg(std::span<const FloatType> data, const FloatType k) {
            namespace hn = hwy::HWY_NAMESPACE;

            static constexpr hn::ScalableTag<FloatType> d;
            static constexpr size_t lanes = hn::MaxLanes(d);

            const auto v_k = hn::Set(d, k);
            auto v_sum = hn::Zero(d);
            auto v_weight_sum = hn::Zero(d);
            size_t i = 0;
            for (; i + lanes <= data.size(); i += lanes) {
                const auto v_x = hn::LoadU(d, data.data() + i);
                const auto v_xk = hn::Mul(v_x, v_k);
                const auto v_weight = hn::Exp(d, v_xk);
                const auto v_weighted_x = hn::Mul(v_weight, v_x);
                v_sum = hn::Add(v_sum, v_weighted_x);
                v_weight_sum = hn::Add(v_weight_sum, v_weight);
            }
            FloatType sum = hn::ReduceSum(d, v_sum);
            FloatType weight_sum = hn::ReduceSum(d, v_weight_sum);
            for (; i < data.size(); ++i) {
                const auto x = data[i];
                const auto weight = std::exp(x * k);
                sum += weight * x;
                weight_sum += weight;
            }
            if (weight_sum < static_cast<FloatType>(1e-10)) {
                return std::numeric_limits<FloatType>::quiet_NaN();
            }
            return sum / weight_sum;
        }
    };
}
