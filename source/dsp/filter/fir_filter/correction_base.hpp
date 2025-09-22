// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "fir_base.hpp"
#include <iostream>

namespace zldsp::filter {
    template<typename FloatType, size_t kDefaultFFTOrder, size_t kStartIdx>
    class CorrectionBase final : public FIRBase<FloatType, kDefaultFFTOrder> {
    public:
        CorrectionBase() : FIRBase<FloatType, kDefaultFFTOrder>() {
        }

        void updateCorrection(std::span<kfr::univector<std::complex<float> > > corrections,
                              const std::span<size_t> on_indices) {
            if (on_indices.empty()) {
                std::fill(total_correction_.begin(), total_correction_.end(), std::complex(1.f, 0.f));
                return;
            }
            // multiply corrections
            bool is_first = true;
            for (const size_t &i: on_indices) {
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
        }

    private:
        kfr::univector<std::complex<float> > total_correction_{};
        size_t multiplication_size_{};

        void setOrder(size_t num_channels, size_t order) override {
            FIRBase<FloatType, kDefaultFFTOrder>::setFFTOrder(num_channels, order);

            total_correction_.resize(FIRBase<FloatType, kDefaultFFTOrder>::num_bin_);
            std::fill(total_correction_.begin(), total_correction_.end(), std::complex(1.f, 0.f));
            if constexpr (kStartIdx != 0) {
                multiplication_size_ = total_correction_.size() - kStartIdx;
            }
        }

        void processSpectrum() override {
            if constexpr (kStartIdx == 0) {
                auto *cdata = reinterpret_cast<std::complex<float> *>(
                    FIRBase<FloatType, kDefaultFFTOrder>::fft_data_.data());
                auto c_vector = kfr::make_univector(cdata, total_correction_.size());
                c_vector = c_vector * total_correction_;
            } else {
                auto *cdata = reinterpret_cast<std::complex<float> *>(
                    FIRBase<FloatType, kDefaultFFTOrder>::fft_data_.data());
                auto c_vector = kfr::make_univector(cdata + kStartIdx, multiplication_size_);
                auto m_vector = kfr::make_univector(total_correction_.data() + kStartIdx, multiplication_size_);
                c_vector = c_vector * m_vector;
            }
        }
    };
}
