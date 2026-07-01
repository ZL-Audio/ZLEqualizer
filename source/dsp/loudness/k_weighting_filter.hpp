// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <numbers>
#include "../filter/iir_filter/tdf/tdf.hpp"

namespace zldsp::loudness {
    /**
     * a K-weighting filter used for integrated loudness measurement
     * @tparam FloatType the float type of input audio buffer
     * @tparam kUseLowPass whether to use an extra lowpass filter at 22,000 Hz
     */
    template <typename FloatType, bool kUseLowPass = false>
    class KWeightingFilter {
    public:
        KWeightingFilter() = default;

        void prepare(const double sample_rate, const size_t num_channels) {
            {
                high_pass_.prepare(sample_rate, num_channels, 0);
                high_pass_.setFilterType(filter::FilterType::kHighPass);
                high_pass_.setOrder(2);
                high_pass_.setFreq(38.13713296248405);
                high_pass_.setQ(0.500242812458813);
                high_pass_.updateCoeffs();
            }
            {
                high_shelf_.prepare(sample_rate, num_channels, 0);
                high_shelf_.setFilterType(filter::FilterType::kHighShelf);
                high_shelf_.setOrder(2);
                high_shelf_.setFreq(1500.6868667368922);
                high_shelf_.setGain(3.9993623475151354);
                high_shelf_.setQ(0.7096433028107384);
                high_shelf_.updateCoeffs();
            }

            if constexpr (kUseLowPass) {
                low_pass_.prepare(sample_rate, num_channels, 0);
                low_pass_.setFilterType(filter::FilterType::kLowPass);
                low_pass_.setOrder(2);
                low_pass_.setFreq(22000.0);
                low_pass_.setQ(0.7071067811865476);
                low_pass_.updateCoeffs();
            }
        }

        void reset() {
            high_pass_.reset();
            high_shelf_.reset();
            if constexpr (kUseLowPass) {
                low_pass_.reset();
            }
        }

        void process(std::span<FloatType*> buffer, const size_t num_samples) {
            high_pass_.process(buffer, num_samples);
            high_shelf_.process(buffer, num_samples);
            if constexpr (kUseLowPass) {
                low_pass_.process(buffer, num_samples);
            }
            for (size_t chan = 0; chan < buffer.size(); chan++) {
                zldsp::vector::multiply(buffer[chan], kBias, num_samples);
            }
        }

    private:
        zldsp::filter::TDF<FloatType, 1> high_pass_, high_shelf_, low_pass_;
        static constexpr FloatType kBias = static_cast<FloatType>(1.0051643348917434);
    };
}
