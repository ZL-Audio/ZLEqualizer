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
#include <vector>
#include <cassert>
#include <cstdint>

namespace zldsp::analyzer {
    class SpectrumAccumulator {
    public:
        SpectrumAccumulator() = default;

        void prepare(const size_t fft_size) {
            sums_.resize(fft_size / 2 + 1);
            reset();
        }

        void reset() {
            sum_count_ = 0;
            std::ranges::fill(sums_, 0.f);
        }

        void process(std::span<float> spectrum_abs_sqr) {
            assert(spectrum_abs_sqr.size() == sums_.size());
            sum_count_ += 1;
            const auto mul = 1.0 / static_cast<double>(sum_count_);
            for (size_t i = 0; i < sums_.size(); ++i) {
                sums_[i] += spectrum_abs_sqr[i];
                spectrum_abs_sqr[i] = static_cast<float>(sums_[i] * mul);
            }
        }

    private:
        std::vector<double> sums_;
        uint64_t sum_count_{0};
    };
}
