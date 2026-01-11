// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../fft/kfr_engine.hpp"
#include "fir_base.hpp"

namespace zldsp::filter {
    template <typename FloatType, size_t kDefaultFFTOrder, size_t kStartIdx>
    class CorrectionBase final : public FIRBase<FloatType, kDefaultFFTOrder> {
    public:
        explicit CorrectionBase(zldsp::fft::KFREngine<float>& fft) : FIRBase<FloatType, kDefaultFFTOrder>(fft) {
        }

        void updateCorrection(std::span<kfr::univector<std::complex<float>>> corrections,
                              const std::span<size_t> on_indices) {
            if (on_indices.empty()) {
                std::fill(total_correction_.begin(), total_correction_.end(), std::complex(1.f, 0.f));
                return;
            }
            // multiply corrections
            bool is_first = true;
            for (const size_t& i : on_indices) {
                if (is_first) {
                    total_correction_ = corrections[i];
                    is_first = false;
                } else {
                    total_correction_ *= corrections[i];
                }
                for (size_t w_idx = kStartIdx; w_idx < total_correction_.size(); ++w_idx) {
                    const auto c_mag = std::abs(total_correction_[w_idx]);
                    // if the total correction is larger than 60 dB, scale back to 60 dB
                    if (c_mag > 1000.f) {
                        total_correction_[w_idx] *= (1000.f / c_mag);
                    }
                }
            }
            // apply the window
            std::copy(total_correction_.begin(), total_correction_.end(), dummy_correction_.begin());
            dummy_correction_[0] = std::complex(dummy_correction_[0].real(), 0.f);
            const auto nyquist_idx = dummy_correction_.size() / 2;
            dummy_correction_[nyquist_idx] = std::complex(dummy_correction_[nyquist_idx].real(), 0.f);
            for (size_t i = 1; i < nyquist_idx; ++i) {
                dummy_correction_[dummy_correction_.size() - i] = std::conj(dummy_correction_[i]);
            }
            FIRBase<FloatType, kDefaultFFTOrder>::fft_.backward(dummy_correction_.data(), fir_coeffs_.data());

            std::rotate(fir_coeffs_.begin(),
                        fir_coeffs_.begin() + static_cast<std::ptrdiff_t>(fir_coeffs_.size() / 2),
                        fir_coeffs_.end());

            for (size_t i = 0; i < fir_coeffs_.size(); ++i) {
                const auto p = static_cast<double>(i) / static_cast<double>(fir_coeffs_.size() - 1);
                const auto window = 0.5 * (1.0 - std::cos(2.0 * std::numbers::pi * p)) / static_cast<double>(fir_coeffs_
                    .size());
                fir_coeffs_[i] *= static_cast<float>(window);
            }
            FIRBase<FloatType, kDefaultFFTOrder>::fft_.forward(fir_coeffs_.data(), dummy_correction_.data());

            std::copy(dummy_correction_.begin(),
                      dummy_correction_.begin() + static_cast<ptrdiff_t>(total_correction_.size()),
                      total_correction_.begin());
            for (size_t i = 0; i < total_correction_.size(); ++i) {
                if (i % 2 == 1) {
                    total_correction_[i] = -total_correction_[i];
                }
            }
            total_correction_.back() = std::complex(std::abs(total_correction_[total_correction_.size() - 2]), 0.f);

            start_idx_ = total_correction_.size() - 1;
            for (size_t i = 0; i < total_correction_.size(); ++i) {
                if (std::abs(total_correction_[i].imag()) > 1e-3
                    || std::abs(total_correction_[i].real() - 1.f) > 1e-3) {
                    start_idx_ = i;
                    break;
                }
            }
        }

    private:
        kfr::univector<std::complex<float>> total_correction_{};

        kfr::univector<std::complex<float>> dummy_correction_{};
        kfr::univector<float> fir_coeffs_{};

        size_t start_idx_{0};

        void setOrder(size_t num_channels, size_t order) override {
            FIRBase<FloatType, kDefaultFFTOrder>::setFFTOrder(num_channels, order);
            fir_coeffs_.resize(FIRBase<FloatType, kDefaultFFTOrder>::fft_size_);
            dummy_correction_.resize(FIRBase<FloatType, kDefaultFFTOrder>::fft_size_);

            total_correction_.resize(FIRBase<FloatType, kDefaultFFTOrder>::num_bin_);
            std::fill(total_correction_.begin(), total_correction_.end(), std::complex(1.f, 0.f));
        }

        void processSpectrum() override {
            auto c_vector = this->fft_data_.slice(start_idx_);
            auto m_vector = total_correction_.slice(start_idx_);
            c_vector = c_vector * m_vector;
        }
    };
}
