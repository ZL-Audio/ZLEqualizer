// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "mag_receiver_base.hpp"
#include "../../chore/decibels.hpp"
#include "../analyzer_base/analyzer_receiver_base.hpp"

namespace zldsp::analyzer {
    namespace hn = hwy::HWY_NAMESPACE;

    class MagReceiver {
    public:
        explicit MagReceiver() = default;

        void run(const zldsp::container::FIFORange range,
                 const std::vector<std::vector<float>>& fifo,
                 const MagType mag_type) {
            dbs_.resize(fifo.size());
            for (size_t chan = 0; chan < fifo.size(); ++chan) {
                if (mag_type == MagType::kRMS) {
                    dbs_[chan] = chore::squareGainToDecibels(MagAnalyzerOps::calculateMS(range, fifo[chan]));
                } else {
                    dbs_[chan] = chore::gainToDecibels(MagAnalyzerOps::calculatePeak(range, fifo[chan]));
                }
            }
        }

        static float calculate(const zldsp::container::FIFORange range,
                               const std::vector<std::vector<float>>& fifo,
                               const MagType mag_type,
                               const StereoType stereo_type) {
            if (range.block_size1 + range.block_size2 == 0) {
                return 0.f;
            }
            if (fifo.size() != 2 || stereo_type == StereoType::kStereo) {
                switch (mag_type) {
                case MagType::kRMS: {
                    float sum_sqr{0.f};
                    for (size_t chan = 0; chan < fifo.size(); ++chan) {
                        sum_sqr += MagAnalyzerOps::calculateMS(range, fifo[chan]);
                    }
                    return chore::squareGainToDecibels(sum_sqr);
                }
                case MagType::kPeak: {
                    float peak{0.f};
                    for (size_t chan = 0; chan < fifo.size(); ++chan) {
                        peak = std::max(peak, MagAnalyzerOps::calculatePeak(range, fifo[chan]));
                    }
                    return chore::gainToDecibels(peak);
                }
                default:
                    return 0.f;
                }
            }
            float value{0.f};
            auto process_segment = [&](const size_t start, const size_t size) {
                if (size == 0) {
                    return;
                }
                switch (stereo_type) {
                case StereoType::kLeft: {
                    if (mag_type == MagType::kPeak) {
                        value = std::max(value, vector::max_abs_of(fifo[0].data() + start, size));
                    } else {
                        value += vector::sum_sqr(fifo[0].data() + start, size);
                    }
                    break;
                }
                case StereoType::kRight: {
                    if (mag_type == MagType::kPeak) {
                        value = std::max(value, vector::max_abs_of(fifo[1].data() + start, size));
                    } else {
                        value += vector::sum_sqr(fifo[1].data() + start, size);
                    }
                    break;
                }
                case StereoType::kMid: {
                    static constexpr hn::ScalableTag<float> d;
                    static constexpr size_t lanes = hn::MaxLanes(d);
                    const auto v_sqrt2_over_2 = hn::Set(d, kSqrt2Over2);
                    const float* __restrict in0 = fifo[0].data() + start;
                    const float* __restrict in1 = fifo[1].data() + start;
                    if (mag_type == MagType::kPeak) {
                        auto v_max_abs = hn::Zero(d);
                        size_t i = 0;
                        for (; i + lanes <= size; i += lanes) {
                            const auto v_in0 = hn::LoadU(d, in0 + i);
                            const auto v_in1 = hn::LoadU(d, in1 + i);
                            const auto v_mid = hn::Mul(v_sqrt2_over_2,hn::Add(v_in0, v_in1));
                            v_max_abs = hn::Max(hn::Abs(v_mid), v_max_abs);
                        }
                        float scalar_max_abs = hn::ReduceMax(d, v_max_abs);
                        for (; i < size; ++i) {
                            const auto v_in0 = in0[i];
                            const auto v_in1 = in1[i];
                            const auto v_mid = kSqrt2Over2 * (v_in0 + v_in1);
                            scalar_max_abs = std::max(std::abs(v_mid), scalar_max_abs);
                        }
                        value = std::max(value, scalar_max_abs);
                    } else {
                        auto v_sum = hn::Zero(d);
                        size_t i = 0;
                        for (; i + lanes <= size; i += lanes) {
                            const auto v_in0 = hn::LoadU(d, in0 + i);
                            const auto v_in1 = hn::LoadU(d, in1 + i);
                            const auto v_mid = hn::Mul(v_sqrt2_over_2,hn::Add(v_in0, v_in1));
                            v_sum = hn::MulAdd(v_mid, v_mid, v_sum);
                        }
                        float scalar_sum = hn::ReduceSum(d, v_sum);
                        for (; i < size; ++i) {
                            const auto v_in0 = in0[i];
                            const auto v_in1 = in1[i];
                            const auto v_mid = kSqrt2Over2 * (v_in0 + v_in1);
                            scalar_sum += v_mid * v_mid;
                        }
                        value += scalar_sum;
                    }
                    break;
                }
                case StereoType::kSide: {
                    static constexpr hn::ScalableTag<float> d;
                    static constexpr size_t lanes = hn::MaxLanes(d);
                    const auto v_sqrt2_over_2 = hn::Set(d, kSqrt2Over2);
                    const float* __restrict in0 = fifo[0].data() + start;
                    const float* __restrict in1 = fifo[1].data() + start;
                    if (mag_type == MagType::kPeak) {
                        auto v_max_abs = hn::Zero(d);
                        size_t i = 0;
                        for (; i + lanes <= size; i += lanes) {
                            const auto v_in0 = hn::LoadU(d, in0 + i);
                            const auto v_in1 = hn::LoadU(d, in1 + i);
                            const auto v_mid = hn::Mul(v_sqrt2_over_2,hn::Sub(v_in0, v_in1));
                            v_max_abs = hn::Max(hn::Abs(v_mid), v_max_abs);
                        }
                        float scalar_max_abs = hn::ReduceMax(d, v_max_abs);
                        for (; i < size; ++i) {
                            const auto v_in0 = in0[i];
                            const auto v_in1 = in1[i];
                            const auto v_mid = kSqrt2Over2 * (v_in0 - v_in1);
                            scalar_max_abs = std::max(std::abs(v_mid), scalar_max_abs);
                        }
                        value = std::max(value, scalar_max_abs);
                    } else {
                        auto v_sum = hn::Zero(d);
                        size_t i = 0;
                        for (; i + lanes <= size; i += lanes) {
                            const auto v_in0 = hn::LoadU(d, in0 + i);
                            const auto v_in1 = hn::LoadU(d, in1 + i);
                            const auto v_mid = hn::Mul(v_sqrt2_over_2,hn::Sub(v_in0, v_in1));
                            v_sum = hn::MulAdd(v_mid, v_mid, v_sum);
                        }
                        float scalar_sum = hn::ReduceSum(d, v_sum);
                        for (; i < size; ++i) {
                            const auto v_in0 = in0[i];
                            const auto v_in1 = in1[i];
                            const auto v_mid = kSqrt2Over2 * (v_in0 - v_in1);
                            scalar_sum += v_mid * v_mid;
                        }
                        value += scalar_sum;
                    }
                    break;
                }
                case StereoType::kStereo:
                default:
                    break;
                }
            };
            process_segment(static_cast<size_t>(range.start_index1), static_cast<size_t>(range.block_size1));
            process_segment(static_cast<size_t>(range.start_index2), static_cast<size_t>(range.block_size2));
            switch (mag_type) {
            case MagType::kRMS: {
                return chore::squareGainToDecibels(value / static_cast<float>(range.block_size1 + range.block_size2));
            }
            case MagType::kPeak: {
                return chore::gainToDecibels(value);
            }
            default:
                return 0.f;
            }
        }

        const std::vector<float>& getDBs() { return dbs_; }

    protected:
        std::vector<float> dbs_;
    };
}
