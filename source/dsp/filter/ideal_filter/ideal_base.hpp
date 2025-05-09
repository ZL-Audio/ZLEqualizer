// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <complex>

namespace zldsp::filter {
    template<typename SampleType>
    class IdealBase {
    public:
        static void updateMagnitude(
            const std::array<double, 6> &coeff,
            const std::vector<SampleType> &ws, std::vector<SampleType> &gains) {
            for (size_t idx = 0; idx < ws.size(); ++idx) {
                gains[idx] *= getMagnitude(coeff, ws[idx]);
            }
        }

        static void updateResponse(
            const std::array<double, 6> &coeff,
            const std::vector<std::complex<SampleType> > &wis, std::vector<std::complex<SampleType> > &response) {
            for (size_t idx = 0; idx < wis.size(); ++idx) {
                response[idx] *= getResponse(coeff, wis[idx]);
            }
        }

        static void updateMixResponse(
            const std::array<double, 6> &coeff,
            const std::vector<std::complex<SampleType> > &wis,
            std::vector<std::complex<SampleType> > &response,
            const size_t startMix, const size_t endMix, const std::vector<SampleType> &mix) {
            for (size_t idx = 0; idx < startMix; ++idx) {
                response[idx] *= getResponse(coeff, wis[idx]);
            }
            for (size_t idx = startMix; idx < endMix; ++idx) {
                auto single_response = getResponse(coeff, wis[idx]);
                single_response = std::polar<SampleType>(std::abs(single_response), std::arg(single_response) * mix[idx]);
                response[idx] *= single_response;
            }
            for (size_t idx = endMix; idx < wis.size(); ++idx) {
                auto single_response = getResponse(coeff, wis[idx]);
                single_response = std::polar<SampleType>(std::abs(single_response), SampleType(0));
                response[idx] *= single_response;
            }
        }

        static SampleType getMagnitude(const std::array<double, 6> &coeff, const SampleType w) {
            const auto w_2 = w * w;
            const auto t1 = coeff[2] - coeff[0] * w_2;
            const auto denominator = coeff[1] * coeff[1] * w_2 + t1 * t1;
            const auto t2 = coeff[5] - coeff[3] * w_2;
            const auto numerator = coeff[4] * coeff[4] * w_2 + t2 * t2;
            return std::sqrt(numerator / denominator);
        }

        // const auto wi = std::complex<SampleType>(SampleType(0), w)
        static std::complex<SampleType> getResponse(const std::array<double, 6> &coeff,
                                                    const std::complex<SampleType> &wi) {
            const auto wi2 = wi * wi;
            return (static_cast<SampleType>(coeff[3]) * wi2 +
                    static_cast<SampleType>(coeff[4]) * wi +
                    static_cast<SampleType>(coeff[5])) / (
                       static_cast<SampleType>(coeff[0]) * wi2 +
                       static_cast<SampleType>(coeff[1]) * wi +
                       static_cast<SampleType>(coeff[2]));
        }
    };
}
