// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "kfr_import.hpp"

namespace zldsp::fft {
    template<typename FloatType>
    class KFREngine {
    public:
        KFREngine() = default;

        void setOrder(const size_t order) {
            fft_size_ = static_cast<size_t>(1) << order;
            fft_plan_ = std::make_unique<kfr::dft_plan_real<FloatType> >(fft_size_);
            temp_buffer_.resize(fft_plan_->temp_size);
        }

        void forward(FloatType *in_buffer, std::complex<FloatType> *out_buffer) {
            fft_plan_->execute(out_buffer, in_buffer, temp_buffer_.data());
        }

        void forward(FloatType *in_buffer, FloatType *float_out_buffer) {
            auto out_buffer = reinterpret_cast<std::complex<FloatType> *>(float_out_buffer);
            forward(in_buffer, out_buffer);
        }

        void backward(std::complex<FloatType> *out_buffer, FloatType *in_buffer) {
            fft_plan_->execute(in_buffer, out_buffer, temp_buffer_.data());
        }

        void backward(FloatType *float_out_buffer, FloatType *in_buffer) {
            auto out_buffer = reinterpret_cast<std::complex<FloatType> *>(float_out_buffer);
            backward(out_buffer, in_buffer);
        }

        void forwardMagnitudeOnly(FloatType *buffer) {
            forward(buffer, buffer);
            auto *out = reinterpret_cast<std::complex<FloatType> *>(buffer);
            for (size_t i = 0; i < (fft_size_ / 2) + 1; ++i) {
                buffer[i] = std::abs(out[i]);
            }
        }

        [[nodiscard]] size_t getSize() const { return fft_size_; }

    private:
        size_t fft_size_{0};
        std::unique_ptr<kfr::dft_plan_real<FloatType> > fft_plan_;
        kfr::univector<kfr::u8> temp_buffer_;
    };
}
