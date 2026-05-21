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

#include "../../compressor/follower/ps_follower.hpp"
#include "../../vector/vector.hpp"
#include "../../container/circular_buffer.hpp"

namespace zldsp::filter {
    /**
     * a dynamic side-chain handler
     * @tparam FloatType the float type of input audio buffer
     */
    template <typename FloatType>
    class DynamicSideHandler {
    public:
        explicit DynamicSideHandler() = default;

        void reset() {
            follower_.reset(static_cast<FloatType>(0));
        }

        void prepare(const double sample_rate, const double rms_max_length_seconds) {
            sample_rate_ = sample_rate;
            follower_.prepare(sample_rate);
            rms_buffer_.setCapacity(static_cast<size_t>(std::round(rms_max_length_seconds * sample_rate_)));
            setRMSLength(rms_length_seconds_);
        }

        /**
         * set dynamic filter base gain
         * @param base_gain
         */
        void setBaseGain(const double base_gain) {
            base_gain_ = base_gain;
            gain_diff_ = target_gain_ - base_gain_;
        }

        /**
         * set dynamic filter target gain
         * @param target_gain
         */
        void setTargetGain(const double target_gain) {
            target_gain_ = target_gain;
            gain_diff_ = target_gain_ - base_gain_;
        }

        /**
         * set dynamic filter threshold
         * @param threshold
         */
        template <bool to_update = true>
        void setThreshold(const FloatType threshold) {
            threshold_ = threshold;
            if constexpr (to_update) {
                updateTK();
            }
        }

        /**
         * set dynamic filter knee width
         * @param knee
         */
        template <bool to_update = true>
        void setKnee(const FloatType knee) {
            knee_ = std::max(knee, static_cast<FloatType>(0.01));
            if constexpr (to_update) {
                updateTK();
            }
        }

        void process(std::span<FloatType*> side_buffer, const size_t num_samples) {
            namespace hn = hwy::HWY_NAMESPACE;
            static constexpr hn::ScalableTag<FloatType> d;
            static constexpr size_t lanes = hn::MaxLanes(d);

            FloatType* out = side_buffer[0];

            const auto v_zero = hn::Zero(d);
            const auto v_one = hn::Set(d, FloatType(1));

            if (use_rms_) {
                // calculate sum of square across channels
                size_t i = 0;
                for (; i + lanes <= num_samples; i += lanes) {
                    auto v = hn::LoadU(d, out + i);
                    v = hn::Mul(v, v);
                    for (size_t chan = 1; chan < side_buffer.size(); ++chan) {
                        auto c = hn::LoadU(d, side_buffer[chan] + i);
                        v = hn::Add(v, hn::Mul(c, c));
                    }
                    hn::StoreU(v, d, out + i);
                }
                for (; i < num_samples; ++i) {
                    FloatType val = out[i] * out[i];
                    for (size_t chan = 1; chan < side_buffer.size(); ++chan) {
                        FloatType c = side_buffer[chan][i];
                        val += c * c;
                    }
                    out[i] = val;
                }
                // add RMS portion
                for (size_t idx = 0; idx < num_samples; ++idx) {
                    if (rms_buffer_.size() > rms_length_counts_) {
                        square_sum_ -= rms_buffer_.popFront();
                    }
                    const auto square = out[idx];
                    square_sum_ += square;
                    rms_buffer_.template pushBack<false>(square);
                    out[idx] = square * rms_mix_c_ + square_sum_ * rms_mix_reverse_;
                }
                // convert to ratio
                const auto v_1e24 = hn::Set(d, FloatType(1e-24));
                const auto v_fma_multiplier = hn::Set(d, slope_sqr_);
                const auto v_fma_offset = hn::Set(d, low_sqr_);
                i = 0;
                for (; i + lanes <= num_samples; i += lanes) {
                    auto v = hn::LoadU(d, out + i);
                    v = hn::Max(v, v_1e24);
                    v = hn::Log(d, v);
                    v = hn::MulAdd(v, v_fma_multiplier, v_fma_offset);
                    v = hn::Clamp(v, v_zero, v_one);
                    hn::StoreU(hn::Mul(v, v), d, out + i);
                }
                for (; i < num_samples; ++i) {
                    FloatType v = std::max(out[i], FloatType(1e-24));
                    v = std::log(v) * slope_sqr_ + low_sqr_;
                    v = std::clamp(v, FloatType(0), FloatType(1));
                    out[i] = v * v;
                }
            } else {
                if (side_buffer.size() == 1) {
                    // convert to ratio
                    const auto v_1e12 = hn::Set(d, FloatType(1e-12));
                    const auto v_fma_multiplier = hn::Set(d, slope_abs_);
                    const auto v_fma_offset = hn::Set(d, low_abs_);
                    size_t i = 0;
                    for (; i + lanes <= num_samples; i += lanes) {
                        auto v = hn::LoadU(d, out + i);
                        v = hn::Max(hn::Abs(v), v_1e12);
                        v = hn::Log(d, v);
                        v = hn::MulAdd(v, v_fma_multiplier, v_fma_offset);
                        v = hn::Clamp(v, v_zero, v_one);
                        hn::StoreU(hn::Mul(v, v), d, out + i);
                    }
                    for (; i < num_samples; ++i) {
                        FloatType v = std::max(std::abs(out[i]), FloatType(1e-12));
                        v = std::log(v) * slope_abs_ + low_abs_;
                        v = std::clamp(v, FloatType(0), FloatType(1));
                        out[i] = v * v;
                    }
                } else {
                    // calculate sum of square across channels and convert to ratio
                    const auto v_1e24 = hn::Set(d, FloatType(1e-24));
                    const auto v_fma_multiplier = hn::Set(d, slope_sqr_);
                    const auto v_fma_offset = hn::Set(d, low_sqr_);
                    size_t i = 0;
                    for (; i + lanes <= num_samples; i += lanes) {
                        auto v = hn::LoadU(d, out + i);
                        v = hn::Mul(v, v);
                        for (size_t chan = 1; chan < side_buffer.size(); ++chan) {
                            auto c = hn::LoadU(d, side_buffer[chan] + i);
                            v = hn::Add(v, hn::Mul(c, c));
                        }
                        v = hn::Max(v, v_1e24);
                        v = hn::Log(d, v);
                        v = hn::MulAdd(v, v_fma_multiplier, v_fma_offset);
                        v = hn::Clamp(v, v_zero, v_one);
                        hn::StoreU(hn::Mul(v, v), d, out + i);
                    }
                    for (; i < num_samples; ++i) {
                        FloatType val = out[i] * out[i];
                        for (size_t chan = 1; chan < side_buffer.size(); ++chan) {
                            FloatType c = side_buffer[chan][i];
                            val += c * c;
                        }
                        FloatType v = std::max(val, FloatType(1e-24));
                        v = std::log(v) * slope_sqr_ + low_sqr_;
                        v = std::clamp(v, FloatType(0), FloatType(1));
                        out[i] = v * v;
                    }
                }
            }
        }

        template <zldsp::compressor::SState s_state>
        void processToGainLinear(FloatType* side, const size_t num_samples, const size_t gain_num) {
            const auto scale = kDbToExp2Sqrt / static_cast<FloatType>(gain_num);
            const auto to_mul = gain_diff_ * scale;
            const auto to_add = base_gain_ * scale;
            for (size_t i = 0; i < num_samples; ++i) {
                side[i] = std::exp2(std::fma(to_mul, follower_.template processSample<s_state>(side[i]), to_add));
            }
        }

        /**
         *
         * @return the underlying follower
         */
        zldsp::compressor::PSFollower<FloatType>& getFollower() {
            return follower_;
        }

        [[nodiscard]] double getBaseGain() const {
            return base_gain_;
        }

        double getCurrentGain() {
            return base_gain_ + follower_.getCurrentSample() * gain_diff_;
        }

        void setRMSLength(const double rms_length_seconds) {
            if (rms_length_seconds > 1e-6) {
                use_rms_ = true;
                rms_length_seconds_ = rms_length_seconds;
                rms_length_counts_ = static_cast<size_t>(std::round(rms_length_seconds_ * sample_rate_));
                rms_mix_reverse_ = rms_mix_ / static_cast<FloatType>(rms_length_counts_);
                if (rms_length_counts_ < rms_buffer_.size()) {
                    const auto num_to_pop = rms_buffer_.size() - rms_length_counts_;
                    for (size_t i = 0; i < num_to_pop; ++i) {
                        square_sum_ -= static_cast<double>(rms_buffer_.popFront());
                    }
                    square_sum_ = std::max(square_sum_, 0.0);
                }
            } else {
                use_rms_ = false;
                rms_buffer_.clear();
                square_sum_ = 0.0;
            }
        }

        void setRMSMix(const FloatType rms_mix) {
            rms_mix_ = rms_mix;
            rms_mix_c_ = FloatType(1) - rms_mix;
            rms_mix_reverse_ = rms_mix_ / static_cast<FloatType>(rms_length_counts_);
        }

    protected:
        static constexpr FloatType kDbToExp2Sqrt = static_cast<FloatType>(0.16609640474436813 * 0.5);

        zldsp::compressor::PSFollower<FloatType> follower_;
        double base_gain_{}, target_gain_{}, gain_diff_{};

        FloatType threshold_{}, knee_{static_cast<FloatType>(0.01)};
        FloatType low_abs_{}, slope_abs_{};
        FloatType low_sqr_{}, slope_sqr_{};

        bool use_rms_{false};
        double sample_rate_{48000.0};
        FloatType rms_length_seconds_{};
        size_t rms_length_counts_{1};
        zldsp::container::CircularBuffer<FloatType> rms_buffer_{1};

        double square_sum_{0.0};

        FloatType rms_mix_{}, rms_mix_c_{}, rms_mix_reverse_{};

        void updateTK() {
            constexpr double kln10 = 2.30258509299404568402;
            constexpr auto inv_ln10_abs = static_cast<FloatType>(20.0 / kln10);
            constexpr auto inv_ln10_sqr = static_cast<FloatType>(10.0 / kln10);
            const auto low = threshold_ - knee_;
            const auto slope = static_cast<FloatType>(0.5) / knee_;
            const auto fma_offset = -(low * slope);

            low_abs_ = fma_offset;
            slope_abs_ = slope * inv_ln10_abs;

            low_sqr_ = fma_offset;
            slope_sqr_ = slope * inv_ln10_sqr;
        }
    };
}
