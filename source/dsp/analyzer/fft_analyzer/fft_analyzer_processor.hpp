// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <complex>
#include "../../fft/zldsp_fft_include.hpp"
#include "../../fft/zldsp_fft_window.hpp"
#include "../../vector/vector.hpp"
#include "../../container/fifo/fifo_base.hpp"
#include "../analyzer_base/analyzer_receiver_base.hpp"

namespace zldsp::analyzer {
    namespace hn = hwy::HWY_NAMESPACE;

    class FFTAnalyzerProcessor {
    public:
        explicit FFTAnalyzerProcessor() = default;

        void prepare(const int fft_order) {
            fft_ = std::make_unique<zldsp::fft::RFFT<float>>(fft_order);
            const auto fft_size = fft_->get_size();

            window_.resize(fft_size);
            zldsp::fft::createPeriodicHanning(std::span{window_.data(), window_.size()});
            const auto scale = 2.f / static_cast<float>(fft_size);
            vector::multiply(window_.data(), scale, window_.size());
            fft_in_.resize(fft_size);
            fft_out_.resize(fft_size / 2 + 1);
        }

        void forwardSqrMag(float* __restrict fft_in, float* __restrict fft_out) const {
            fft_->forward_sqr_mag(fft_in, fft_out);
        }

        vector::aligned_vector<float>& getFFTIn() {
            return fft_in_;
        }

        vector::aligned_vector<float>& getFFTOut() {
            return fft_out_;
        }

        vector::aligned_vector<float>& getWindow() {
            return window_;
        }

        [[nodiscard]] size_t getFFTSize() const {
            return fft_in_.size();
        }

    private:
        vector::aligned_vector<float> fft_in_;
        vector::aligned_vector<float> fft_out_;

        std::unique_ptr<zldsp::fft::RFFT<float>> fft_;
        vector::aligned_vector<float> window_;
    };
}
