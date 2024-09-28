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
        IdealBase() = default;

        void updateFromIdeal(const std::array<double, 6> &coeffs) {
            coeff = coeffs;
        }

        void updateMagnitude(const std::vector<SampleType> &ws, std::vector<SampleType> &gains) {
            for (size_t idx = 0; idx < ws.size(); ++idx) {
                gains[idx] *= getMagnitude(ws[idx]);
            }
        }

        void updateResponse(const std::vector<SampleType> &ws, std::vector<SampleType> &response) {
            for (size_t idx = 0; idx < ws.size(); ++idx) {
                response[idx] *= getResponse(ws[idx]);
            }
        }

        SampleType getMagnitude(const SampleType w) {
            const auto w_2 = w * w;
            const auto t1 = coeff[2] - coeff[0] * w_2;
            const auto denominator = coeff[1] * coeff[1] * w_2 + t1 * t1;
            const auto t2 = coeff[5] - coeff[3] * w_2;
            const auto numerator = coeff[4] * coeff[4] * w_2 + t2 * t2;
            return std::sqrt(numerator / denominator);
        }

        std::complex<SampleType> getResponse(const SampleType w) {
            const auto wi = std::complex<SampleType>(SampleType(0), w);
            const auto wi2 = wi * wi;
            return (coeff[3] * wi2 + coeff[4] * wi + coeff[5]) / (coeff[0] * wi2 + coeff[1] * w + coeff[2]);
        }

    private:
        std::array<SampleType, 6> coeff{};
    };
}

#endif //ZLFILTER_IDEAL_BASE_HPP
