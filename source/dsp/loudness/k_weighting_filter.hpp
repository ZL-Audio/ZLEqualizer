// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <numbers>
#include "../filter/filter.hpp"

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
            high_pass_.prepare(num_channels);
            high_shelf_.prepare(num_channels);
            const auto scale = 2.0 * std::numbers::pi / sample_rate;
            const auto w1 = scale * 38.13713296248405;
            const auto w2 = scale * 1500.6868667368922;
            high_pass_.updateFromBiquad(
                zldsp::filter::MartinCoeff::get2HighPass(w1, 0.500242812458813));
            high_shelf_.updateFromBiquad(
                zldsp::filter::MartinCoeff::get2HighShelf(w2, 1.5847768458311522, 0.7096433028107384));
            if constexpr (kUseLowPass) {
                low_pass_.prepare(num_channels);
                const auto w3 = scale * 22000.0;
                low_pass_.updateFromBiquad(
                    zldsp::filter::MartinCoeff::get2LowPass(w3, 0.7071067811865476));
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
            for (size_t channel = 0; channel < buffer.size(); ++channel) {
                auto samples = buffer[channel];
                for (size_t i = 0; i < num_samples; ++i) {
                    auto sample = samples[i];
                    sample = high_pass_.processSample(channel, sample);
                    sample = high_shelf_.processSample(channel, sample);
                    if constexpr (kUseLowPass) {
                        sample = low_pass_.processSample(channel, sample);
                    }
                    samples[i] = sample;
                }
            }
            for (size_t channel = 0; channel < buffer.size(); ++channel) {
                auto v = kfr::make_univector(buffer[channel], num_samples);
                v = v * kBias;
            }
        }

    private:
        zldsp::filter::TDFBase<FloatType> high_pass_, high_shelf_, low_pass_;
        static constexpr FloatType kBias = static_cast<FloatType>(1.0051643348917434);
    };
}
