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
#include <vector>
#include <complex>
#include <span>

namespace zldsp::filter {
    template <typename FloatType>
    class TDFBase {
    public:
        // w should be an array of std::exp(-2pi * f / samplerate * i)
        static void updateResponse(
            const std::array<double, 6>& coeff,
            const std::vector<std::complex<FloatType>>& wis, std::vector<std::complex<FloatType>>& response) {
            for (size_t idx = 0; idx < wis.size(); ++idx) {
                response[idx] *= getResponse(coeff, wis[idx]);
            }
        }

        // w should be std::exp(-2pi * f / samplerate * i)
        // const auto wi = std::exp(std::complex<FloatType>(FloatType(0), w))
        static std::complex<FloatType> getResponse(const std::array<double, 6>& coeff,
                                                   const std::complex<FloatType>& wi) {
            const auto wi2 = wi * wi;
            return (static_cast<FloatType>(coeff[3]) +
                static_cast<FloatType>(coeff[4]) * wi +
                static_cast<FloatType>(coeff[5]) * wi2) / (
                static_cast<FloatType>(coeff[0]) +
                static_cast<FloatType>(coeff[1]) * wi +
                static_cast<FloatType>(coeff[2]) * wi2);
        }

        TDFBase() = default;

        void prepare(const size_t num_channels) {
            s1_.resize(num_channels);
            s2_.resize(num_channels);
            reset();
        }

        void reset() {
            std::fill(s1_.begin(), s1_.end(), static_cast<FloatType>(0));
            std::fill(s2_.begin(), s2_.end(), static_cast<FloatType>(0));
        }

        template <bool bypass = false>
        void process(std::span<FloatType*> buffer, const size_t num_samples) noexcept {
            for (size_t channel = 0; channel < buffer.size(); ++channel) {
                auto* samples = buffer[channel];
                for (size_t i = 0; i < num_samples; ++i) {
                    if constexpr (bypass) {
                        processSample(channel, samples[i]);
                    } else {
                        samples[i] = processSample(channel, samples[i]);
                    }
                }
            }
        }

        FloatType processSample(const size_t channel, FloatType input) {
            const auto output = input * coeff_[0] + s1_[channel];
            s1_[channel] = (input * coeff_[1]) - (output * coeff_[3]) + s2_[channel];
            s2_[channel] = (input * coeff_[2]) - (output * coeff_[4]);
            return output;
        }

        void updateFromBiquad(const std::array<double, 6>& coeff) {
            const auto a0_inv = 1.0 / coeff[0];
            coeff_[0] = static_cast<FloatType>(coeff[3] * a0_inv);
            coeff_[1] = static_cast<FloatType>(coeff[4] * a0_inv);
            coeff_[2] = static_cast<FloatType>(coeff[5] * a0_inv);
            coeff_[3] = static_cast<FloatType>(coeff[1] * a0_inv);
            coeff_[4] = static_cast<FloatType>(coeff[2] * a0_inv);
        }

    private:
        std::array<FloatType, 5> coeff_{0, 0, 0, 0, 0};
        std::vector<FloatType> s1_, s2_;
    };
}
