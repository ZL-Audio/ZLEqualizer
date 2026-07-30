// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>
#include <memory>
#include <span>
#include <algorithm>
#include <cmath>

#include "../dsp/fft/zldsp_fft_include.hpp"
#include "../dsp/vector/vector.hpp"
#include "sample_rate_helper.hpp"

namespace zlp {
    namespace hn = hwy::HWY_NAMESPACE;

    template<typename FloatType>
    class StereoFIRProcessor {
    public:
        StereoFIRProcessor(std::unique_ptr<zldsp::fft::RFFT<float>> &fft,
                           const size_t default_fft_order, const size_t start_idx)
            : fft_(fft), fft_order_(0), fft_size_(0), num_bin_(0), hop_size_(0),
              default_fft_order_(default_fft_order),
              start_idx_(start_idx) {
        }

        void prepare(const double sample_rate) {
            setOrder(zlp::getScaledOrder(sample_rate, default_fft_order_));
            reset();
        }

        void setOrder(const size_t order) {
            fft_order_ = order;
            fft_size_ = static_cast<size_t>(1) << fft_order_;
            num_bin_ = fft_size_ / 2 + 1;
            hop_size_ = fft_size_ / overlap_;
            latency_ = static_cast<int>(fft_size_);

            fft_ = std::make_unique<zldsp::fft::RFFT<float>>(fft_order_);

            window1_.resize(fft_size_);
            window2_.resize(fft_size_);
            window_bypass_.resize(fft_size_);
            zldsp::fft::createPeriodicHanning<float>(window1_, 2.f / static_cast<float>(fft_size_));
            const auto v_window2_scale = hn::Set(d, static_cast<float>(fft_size_) / 3.f);
            const auto v_bypass_scale = hn::Set(d, static_cast<float>(fft_size_ * fft_size_) / 6.f);
            for (size_t i = 0; i < fft_size_; i += lanes) {
                const auto v_window1 = hn::Load(d, window1_.data() + i);
                const auto v_window2 = hn::Mul(v_window1, v_window2_scale);
                hn::Store(v_window2, d, window2_.data() + i);
                const auto v_window_bypass = hn::Mul(hn::Mul(v_window1, v_window1), v_bypass_scale);
                hn::Store(v_window_bypass, d, window_bypass_.data() + i);
            }

            for (auto &fifo: input_fifo_) fifo.resize(fft_size_);
            for (auto &fifo: output_fifo_) fifo.resize(fft_size_);

            for (auto &buf: fft_in_) buf.resize(fft_size_);
            for (auto &buf: fft_out_real_) buf.resize(num_bin_);
            for (auto &buf: fft_out_imag_) buf.resize(num_bin_);

            for (auto &buf: correction_real_) {
                buf.resize(num_bin_);
                std::ranges::fill(buf, 1.f);
            }
            for (auto &buf: correction_imag_) {
                buf.resize(num_bin_);
                std::ranges::fill(buf, 0.f);
            }
        }

        void reset() {
            pos_ = 0;
            count_ = 0;
            for (auto &fifo: input_fifo_) {
                std::ranges::fill(fifo, 0.f);
            }
            for (auto &fifo: output_fifo_) {
                std::ranges::fill(fifo, 0.f);
            }
            for (auto &buf: fft_in_) {
                std::ranges::fill(buf, 0.f);
            }
        }

        void updateCorrection(std::span<zldsp::vector::aligned_vector<float>> calculators_real,
                              std::span<zldsp::vector::aligned_vector<float>> calculators_imag,
                              const std::array<std::vector<size_t>, 5> &on_indices) {
            for (size_t type = 0; type < 5; ++type) {
                if (on_indices[type].empty()) {
                    std::ranges::fill(correction_real_[type], 1.f);
                    std::ranges::fill(correction_imag_[type], 0.f);
                    continue;
                }
                bool is_first = true;
                for (const size_t &idx: on_indices[type]) {
                    if (is_first) {
                        zldsp::vector::copy(correction_real_[type].data(), calculators_real[idx].data(), num_bin_);
                        zldsp::vector::copy(correction_imag_[type].data(), calculators_imag[idx].data(), num_bin_);
                        is_first = false;
                    } else {
                        for (size_t i = 0; i < num_bin_ - 1; i += lanes) {
                            const auto t_real_v = hn::Load(d, calculators_real[idx].data() + i);
                            const auto t_imag_v = hn::Load(d, calculators_imag[idx].data() + i);
                            const auto cor_real_v = hn::Load(d, correction_real_[type].data() + i);
                            const auto cor_imag_v = hn::Load(d, correction_imag_[type].data() + i);

                            const auto out_real_v = hn::NegMulAdd(t_imag_v, cor_imag_v, hn::Mul(t_real_v, cor_real_v));
                            const auto out_imag_v = hn::MulAdd(t_real_v, cor_imag_v, hn::Mul(t_imag_v, cor_real_v));

                            hn::Store(out_real_v, d, correction_real_[type].data() + i);
                            hn::Store(out_imag_v, d, correction_imag_[type].data() + i);
                        }
                        const auto nyq_t_real = calculators_real[idx].back();
                        const auto nyq_t_imag = calculators_imag[idx].back();
                        const auto nyq_c_real = correction_real_[type].back();
                        const auto nyq_c_imag = correction_imag_[type].back();
                        correction_real_[type].back() = nyq_c_real * nyq_t_real - nyq_c_imag * nyq_t_imag;
                        correction_imag_[type].back() = nyq_c_real * nyq_t_imag + nyq_c_imag * nyq_t_real;
                    }
                    for (size_t w_idx = start_idx_; w_idx < num_bin_; ++w_idx) {
                        const auto re = correction_real_[type][w_idx];
                        const auto im = correction_imag_[type][w_idx];
                        if (const auto abs_sqr = re * re + im * im; abs_sqr > 1e6f) {
                            const auto scale = 1000.f / std::sqrt(abs_sqr);
                            correction_real_[type][w_idx] *= scale;
                            correction_imag_[type][w_idx] *= scale;
                        }
                    }
                }
                const auto last_real = correction_real_[type].back();
                const auto last_imag = correction_imag_[type].back();
                const auto last_abs = std::sqrt(last_real * last_real + last_imag * last_imag);
                correction_real_[type].back() = last_real > 0.f ? last_abs : -last_abs;
                correction_imag_[type].back() = 0.f;
            }
        }

        template<bool has_stereo, bool has_l, bool has_r, bool has_m, bool has_s>
        void process(std::span<FloatType *> buffer, const size_t num_samples, const bool bypass) {
            for (size_t i = 0; i < num_samples; ++i) {
                for (size_t chan = 0; chan < 2; ++chan) {
                    input_fifo_[chan][pos_] = static_cast<float>(buffer[chan][i]);
                    buffer[chan][i] = static_cast<FloatType>(output_fifo_[chan][pos_]);
                    output_fifo_[chan][pos_] = 0.f;
                }
                pos_ += 1;
                if (pos_ == fft_size_) pos_ = 0;
                count_ += 1;
                if (count_ == hop_size_) {
                    count_ = 0;
                    processFrame<has_stereo, has_l, has_r, has_m, has_s>(bypass);
                }
            }
        }

        [[nodiscard]] int getLatency() const { return latency_; }

        [[nodiscard]] size_t getNumBin() const { return num_bin_; }

    private:
        static constexpr hn::ScalableTag<float> d;
        static constexpr size_t lanes = hn::MaxLanes(d);

        std::unique_ptr<zldsp::fft::RFFT<float>> &fft_;
        zldsp::vector::aligned_vector<float> window1_, window2_, window_bypass_;

        size_t fft_order_, fft_size_, num_bin_, hop_size_;
        size_t default_fft_order_, start_idx_;
        size_t overlap_ = 4;
        static constexpr float kWindowCorrection = 2.0f / 3.0f;

        size_t count_ = 0;
        size_t pos_ = 0;

        std::array<zldsp::vector::aligned_vector<float>, 2> input_fifo_, output_fifo_;
        std::array<zldsp::vector::aligned_vector<float>, 2> fft_in_;
        std::array<zldsp::vector::aligned_vector<float>, 2> fft_out_real_, fft_out_imag_;

        std::array<zldsp::vector::aligned_vector<float>, 5> correction_real_, correction_imag_;

        int latency_{0};

        template<bool has_stereo, bool has_l, bool has_r, bool has_m, bool has_s>
        void processFrame(const bool bypass) {
            for (size_t chan = 0; chan < 2; ++chan) {
                zldsp::vector::copy(fft_in_[chan].data(), input_fifo_[chan].data() + pos_, fft_size_ - pos_);
                if (pos_ > 0) {
                    zldsp::vector::copy(fft_in_[chan].data() + fft_size_ - pos_, input_fifo_[chan].data(), pos_);
                }
            }

            if (!bypass) {
                multiplyWithWindow(fft_in_[0].data(), fft_in_[1].data(), window1_.data());
                for (size_t chan = 0; chan < 2; ++chan) {
                    fft_->forward(fft_in_[chan].data(), {fft_out_real_[chan].data(), fft_out_imag_[chan].data()}); // NOLINT
                }

                processSpectrum<has_stereo, has_l, has_r, has_m, has_s>();

                for (size_t chan = 0; chan < 2; ++chan) {
                    fft_->backward({fft_out_real_[chan].data(), fft_out_imag_[chan].data()}, fft_in_[chan].data()); // NOLINT
                }
                multiplyWithWindow(fft_in_[0].data(), fft_in_[1].data(), window2_.data());
            } else {
                multiplyWithWindow(fft_in_[0].data(), fft_in_[1].data(), window_bypass_.data());
            }

            for (size_t chan = 0; chan < 2; ++chan) {
                for (size_t i = 0; i < pos_; ++i) {
                    output_fifo_[chan][i] += fft_in_[chan][i + fft_size_ - pos_];
                }
                for (size_t i = 0; i < fft_size_ - pos_; ++i) {
                    output_fifo_[chan][i + pos_] += fft_in_[chan][i];
                }
            }
        }

        template<bool has_stereo, bool has_l, bool has_r, bool has_m, bool has_s>
        void processSpectrum() {
            for (size_t i = 0; i < num_bin_ - 1; i += lanes) {
                auto l_real = hn::Load(d, fft_out_real_[0].data() + i);
                auto l_imag = hn::Load(d, fft_out_imag_[0].data() + i);
                auto r_real = hn::Load(d, fft_out_real_[1].data() + i);
                auto r_imag = hn::Load(d, fft_out_imag_[1].data() + i);

                if constexpr (has_stereo) {
                    const auto c_st_r = hn::Load(d, correction_real_[0].data() + i);
                    const auto c_st_i = hn::Load(d, correction_imag_[0].data() + i);

                    const auto next_l_real = hn::NegMulAdd(l_imag, c_st_i, hn::Mul(l_real, c_st_r));
                    const auto next_l_imag = hn::MulAdd(l_real, c_st_i, hn::Mul(l_imag, c_st_r));
                    const auto next_r_real = hn::NegMulAdd(r_imag, c_st_i, hn::Mul(r_real, c_st_r));
                    const auto next_r_imag = hn::MulAdd(r_real, c_st_i, hn::Mul(r_imag, c_st_r));
                    l_real = next_l_real;
                    l_imag = next_l_imag;
                    r_real = next_r_real;
                    r_imag = next_r_imag;
                }

                if constexpr (has_l) {
                    const auto c_l_r = hn::Load(d, correction_real_[1].data() + i);
                    const auto c_l_i = hn::Load(d, correction_imag_[1].data() + i);
                    const auto next_l_real = hn::NegMulAdd(l_imag, c_l_i, hn::Mul(l_real, c_l_r));
                    const auto next_l_imag = hn::MulAdd(l_real, c_l_i, hn::Mul(l_imag, c_l_r));
                    l_real = next_l_real;
                    l_imag = next_l_imag;
                }

                if constexpr (has_r) {
                    const auto c_r_r = hn::Load(d, correction_real_[2].data() + i);
                    const auto c_r_i = hn::Load(d, correction_imag_[2].data() + i);
                    const auto next_r_real = hn::NegMulAdd(r_imag, c_r_i, hn::Mul(r_real, c_r_r));
                    const auto next_r_imag = hn::MulAdd(r_real, c_r_i, hn::Mul(r_imag, c_r_r));
                    r_real = next_r_real;
                    r_imag = next_r_imag;
                }

                if constexpr (has_m || has_s) {
                    const auto half = hn::Set(d, 0.5f);

                    auto m_real = hn::Mul(half, hn::Add(l_real, r_real));
                    auto m_imag = hn::Mul(half, hn::Add(l_imag, r_imag));
                    auto s_real = hn::Mul(half, hn::Sub(l_real, r_real));
                    auto s_imag = hn::Mul(half, hn::Sub(l_imag, r_imag));

                    if constexpr (has_m) {
                        const auto c_m_r = hn::Load(d, correction_real_[3].data() + i);
                        const auto c_m_i = hn::Load(d, correction_imag_[3].data() + i);
                        const auto next_m_real = hn::NegMulAdd(m_imag, c_m_i, hn::Mul(m_real, c_m_r));
                        const auto next_m_imag = hn::MulAdd(m_real, c_m_i, hn::Mul(m_imag, c_m_r));
                        m_real = next_m_real;
                        m_imag = next_m_imag;
                    }

                    if constexpr (has_s) {
                        const auto c_s_r = hn::Load(d, correction_real_[4].data() + i);
                        const auto c_s_i = hn::Load(d, correction_imag_[4].data() + i);
                        const auto next_s_real = hn::NegMulAdd(s_imag, c_s_i, hn::Mul(s_real, c_s_r));
                        const auto next_s_imag = hn::MulAdd(s_real, c_s_i, hn::Mul(s_imag, c_s_r));
                        s_real = next_s_real;
                        s_imag = next_s_imag;
                    }

                    l_real = hn::Add(m_real, s_real);
                    l_imag = hn::Add(m_imag, s_imag);
                    r_real = hn::Sub(m_real, s_real);
                    r_imag = hn::Sub(m_imag, s_imag);
                }

                hn::Store(l_real, d, fft_out_real_[0].data() + i);
                hn::Store(l_imag, d, fft_out_imag_[0].data() + i);
                hn::Store(r_real, d, fft_out_real_[1].data() + i);
                hn::Store(r_imag, d, fft_out_imag_[1].data() + i);
            }

            {
                auto l_real = fft_out_real_[0].back();
                auto l_imag = fft_out_imag_[0].back();
                auto r_real = fft_out_real_[1].back();
                auto r_imag = fft_out_imag_[1].back();

                if constexpr (has_stereo) {
                    const auto c_st_r = correction_real_[0].back();
                    const auto c_st_i = correction_imag_[0].back();
                    const auto next_l_real = l_real * c_st_r - l_imag * c_st_i;
                    const auto next_l_imag = l_real * c_st_i + l_imag * c_st_r;
                    const auto next_r_real = r_real * c_st_r - r_imag * c_st_i;
                    const auto next_r_imag = r_real * c_st_i + r_imag * c_st_r;
                    l_real = next_l_real;
                    l_imag = next_l_imag;
                    r_real = next_r_real;
                    r_imag = next_r_imag;
                }

                if constexpr (has_l) {
                    const auto c_l_r = correction_real_[1].back();
                    const auto c_l_i = correction_imag_[1].back();
                    const auto next_l_real = l_real * c_l_r - l_imag * c_l_i;
                    const auto next_l_imag = l_real * c_l_i + l_imag * c_l_r;
                    l_real = next_l_real;
                    l_imag = next_l_imag;
                }

                if constexpr (has_r) {
                    const auto c_r_r = correction_real_[2].back();
                    const auto c_r_i = correction_imag_[2].back();
                    const auto next_r_real = r_real * c_r_r - r_imag * c_r_i;
                    const auto next_r_imag = r_real * c_r_i + r_imag * c_r_r;
                    r_real = next_r_real;
                    r_imag = next_r_imag;
                }

                if constexpr (has_m || has_s) {
                    auto m_real = 0.5f * (l_real + r_real);
                    auto m_imag = 0.5f * (l_imag + r_imag);
                    auto s_real = 0.5f * (l_real - r_real);
                    auto s_imag = 0.5f * (l_imag - r_imag);

                    if constexpr (has_m) {
                        const auto c_m_r = correction_real_[3].back();
                        const auto c_m_i = correction_imag_[3].back();
                        const auto next_m_real = m_real * c_m_r - m_imag * c_m_i;
                        const auto next_m_imag = m_real * c_m_i + m_imag * c_m_r;
                        m_real = next_m_real;
                        m_imag = next_m_imag;
                    }

                    if constexpr (has_s) {
                        const auto c_s_r = correction_real_[4].back();
                        const auto c_s_i = correction_imag_[4].back();
                        const auto next_s_real = s_real * c_s_r - s_imag * c_s_i;
                        const auto next_s_imag = s_real * c_s_i + s_imag * c_s_r;
                        s_real = next_s_real;
                        s_imag = next_s_imag;
                    }

                    l_real = m_real + s_real;
                    l_imag = m_imag + s_imag;
                    r_real = m_real - s_real;
                    r_imag = m_imag - s_imag;
                }

                fft_out_real_[0].back() = l_real;
                fft_out_imag_[0].back() = 0.f;
                fft_out_real_[1].back() = r_real;
                fft_out_imag_[1].back() = 0.f;
            }
        }

        void multiplyWithWindow(float * HWY_RESTRICT in1_ptr,
                                float * HWY_RESTRICT in2_ptr,
                                const float * HWY_RESTRICT window_ptr) const {
            for (size_t i = 0; i < fft_size_; i += lanes) {
                const auto v_window = hn::Load(d, window_ptr + i);
                const auto v_in1 = hn::Load(d, in1_ptr + i);
                const auto v_in2 = hn::Load(d, in2_ptr + i);
                hn::Store(hn::Mul(v_window, v_in1), d, in1_ptr + i);
                hn::Store(hn::Mul(v_window, v_in2), d, in2_ptr + i);
            }
        }
    };
}
