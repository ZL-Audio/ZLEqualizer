// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../fft/zldsp_fft_window.hpp"
#include "fir_base.hpp"

namespace zldsp::filter {
    namespace hn = hwy::HWY_NAMESPACE;

    template <typename FloatType, size_t kDefaultFFTOrder, size_t kStartIdx>
    class CorrectionBase final : public FIRBase<FloatType, kDefaultFFTOrder> {
    public:
        explicit CorrectionBase(std::unique_ptr<zldsp::fft::RFFT<float>>& fft) :
            FIRBase<FloatType, kDefaultFFTOrder>(fft) {
        }

        void updateCorrection(std::span<vector::aligned_vector<float>> corrections_real,
                              std::span<vector::aligned_vector<float>> corrections_imag,
                              const std::span<size_t> on_indices) {
            if (on_indices.empty()) {
                std::ranges::fill(correction_real_, 1.f);
                std::ranges::fill(correction_imag_, 0.f);
                return;
            }
            // multiply corrections
            bool is_first = true;
            for (const size_t& idx : on_indices) {
                if (is_first) {
                    vector::copy(correction_real_.data(), corrections_real[idx].data(), correction_real_.size());
                    vector::copy(correction_imag_.data(), corrections_imag[idx].data(), correction_imag_.size());
                    is_first = false;
                } else {
                    static constexpr hn::ScalableTag<float> d;
                    static constexpr size_t lanes = hn::MaxLanes(d);
                    for (size_t i = 0; i < correction_real_.size() - 1; i += lanes) {
                        const auto t_real_v = hn::Load(d, corrections_real[idx].data() + i);
                        const auto t_imag_v = hn::Load(d, corrections_imag[idx].data() + i);
                        const auto cor_real_v = hn::Load(d, correction_real_.data() + i);
                        const auto cor_imag_v = hn::Load(d, correction_imag_.data() + i);

                        const auto out_real_v = hn::NegMulAdd(t_imag_v, cor_imag_v, hn::Mul(t_real_v, cor_real_v));
                        const auto out_imag_v = hn::MulAdd(t_real_v, cor_imag_v, hn::Mul(t_imag_v, cor_real_v));

                        hn::Store(out_real_v, d, correction_real_.data() + i);
                        hn::Store(out_imag_v, d, correction_imag_.data() + i);
                    }
                }
                for (size_t w_idx = kStartIdx; w_idx < correction_real_.size(); ++w_idx) {
                    const auto re = correction_real_[w_idx];
                    const auto im = correction_imag_[w_idx];
                    // if the total correction is larger than 60 dB, scale back to 60 dB
                    if (const auto abs_sqr = re * re + im * im; abs_sqr > 1e6f) {
                        const auto scale = 1000.f / std::sqrt(abs_sqr);
                        correction_real_[w_idx] *= scale;
                        correction_imag_[w_idx] *= scale;
                    }
                }
            }
            {
                const auto last_real = correction_real_.back();
                const auto last_imag = correction_imag_.back();
                const auto last_abs = std::sqrt(last_real * last_real + last_imag * last_imag);
                if (last_real > 0.f) {
                    correction_real_.back() = last_abs;
                } else {
                    correction_real_.back() = -last_abs;
                }
            }
        }

    private:
        vector::aligned_vector<float> correction_real_, correction_imag_;

        void setOrder(size_t num_channels, size_t order) override {
            FIRBase<FloatType, kDefaultFFTOrder>::setFFTOrder(num_channels, order);

            correction_real_.resize(FIRBase<FloatType, kDefaultFFTOrder>::num_bin_);
            correction_imag_.resize(FIRBase<FloatType, kDefaultFFTOrder>::num_bin_);
            std::ranges::fill(correction_real_, 1.f);
            std::ranges::fill(correction_imag_, 0.f);
        }

        void processSpectrum() override {
            static constexpr hn::ScalableTag<float> d;
            static constexpr size_t lanes = hn::MaxLanes(d);
            for (size_t i = 0; i < correction_real_.size() - 1; i += lanes) {
                const auto fft_real_v = hn::Load(d, this->fft_out_real_.data() + i);
                const auto fft_imag_v = hn::Load(d, this->fft_out_imag_.data() + i);
                const auto cor_real_v = hn::Load(d, correction_real_.data() + i);
                const auto cor_imag_v = hn::Load(d, correction_imag_.data() + i);

                const auto out_real_v = hn::NegMulAdd(fft_imag_v, cor_imag_v, hn::Mul(fft_real_v, cor_real_v));
                const auto out_imag_v = hn::MulAdd(fft_real_v, cor_imag_v, hn::Mul(fft_imag_v, cor_real_v));

                hn::Store(out_real_v, d, this->fft_out_real_.data() + i);
                hn::Store(out_imag_v, d, this->fft_out_imag_.data() + i);
            }
            this->fft_out_real_.back() *= correction_real_.back();
        }
    };
}
