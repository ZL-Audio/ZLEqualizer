// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "multiple_fft_base.hpp"

namespace zldsp::analyzer {
    /**
     * a fft analyzer which make sure that multiple FFTs are synchronized in time
     * and can create corresponding analyzer paths
     * @tparam FloatType the float type of input audio buffers
     * @tparam FFTNum the number of FFTs
     * @tparam PointNum the number of output points
     */
    template<typename FloatType, size_t FFTNum, size_t PointNum>
    class MultipleFFTAnalyzer final : public MultipleFFTBase<FloatType, FFTNum, PointNum> {
    public:
        explicit MultipleFFTAnalyzer(const size_t fft_order = 12)
            : MultipleFFTBase<FloatType, FFTNum, PointNum>(fft_order) {
        }

        ~MultipleFFTAnalyzer() = default;

        /**
         * create path x coordinate
         * @param xs
         * @param width
         */
        void createPathXs(std::span<float> xs, const float width) {
            for (size_t idx = 0; idx < this->interplot_freqs_p_.size(); ++idx) {
                xs[idx] = this->interplot_freqs_p_[idx] * width;
            }
        }

        /**
         * create path y coordinate
         * @param ys
         * @param height
         * @param min_db
         */
        void createPathYs(std::array<std::span<float>, FFTNum> ys, const float height, const float min_db = -72.f) {
            std::vector<size_t> is_on_vector{};
            for (size_t i = 0; i < FFTNum; ++i) {
                if (this->is_on_[i].load(std::memory_order::relaxed)) {
                    is_on_vector.push_back(i);
                }
            }
            const auto scale = height / min_db;
            for (const auto &i: is_on_vector) {
                if (!ys[i].empty()) {
                    auto db = kfr::make_univector(this->result_dbs_[i]);
                    auto y = kfr::make_univector(ys[i]);
                    y = db * scale;
                }
            }
        }
    };
}
