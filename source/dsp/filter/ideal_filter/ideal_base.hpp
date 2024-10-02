// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLFILTER_IDEAL_BASE_HPP
#define ZLFILTER_IDEAL_BASE_HPP

#include <array>
#include <complex>

namespace zlFilter {
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
            return (coeff[3] * wi2 + coeff[4] * wi + coeff[5]) / (coeff[0] * wi2 + coeff[1] * wi + coeff[2]);
        }
    };
}

#endif //ZLFILTER_IDEAL_BASE_HPP
