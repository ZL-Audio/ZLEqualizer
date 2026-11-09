// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <vector>
#include "coeff.hpp"

namespace zldsp::filter::scaled_pass {
    template <bool svf>
    class State {
    public:
        void prepare(const size_t channels) {
            first_.assign(channels * Coefficients::kMaxFirst, 0.0);
            s1_.assign(channels * Coefficients::kMaxSecond, 0.0);
            s2_.assign(channels * Coefficients::kMaxSecond, 0.0);
        }

        void reset() {
            std::ranges::fill(first_, 0.0);
            std::ranges::fill(s1_, 0.0);
            std::ranges::fill(s2_, 0.0);
        }

        double process(const Coefficients& coefficients, const size_t channel, double sample) {
            for (size_t i = 0; i < coefficients.first_count; ++i) {
                const auto& c = coefficients.first[i];
                auto& state = first_[channel * Coefficients::kMaxFirst + i];
                if constexpr (svf) {
                    const auto theta = c[1] * (sample - state);
                    sample = c[0] * theta + c[2] * state;
                    state += theta;
                } else {
                    const auto output = c[1] * sample + state;
                    state += c[2] * sample - c[0] * output;
                    sample = output;
                }
            }
            for (size_t i = 0; i < coefficients.second_count; ++i) {
                const auto& c = coefficients.second[i];
                auto& s1 = s1_[channel * Coefficients::kMaxSecond + i];
                auto& s2 = s2_[channel * Coefficients::kMaxSecond + i];
                if constexpr (svf) {
                    const auto theta = c[1] * (sample - s1 - c[2] * s2);
                    sample = c[0] * theta + c[3] * s2 + c[4] * s1;
                    s1 += theta;
                    s2 = -s2 - c[2] * theta;
                } else {
                    const auto output = sample * c[2] + s1;
                    s1 = sample * c[3] - output * c[0] + s2;
                    s2 = sample * c[4] - output * c[1];
                    sample = output;
                }
            }
            return sample;
        }

    private:
        std::vector<double> first_{}, s1_{}, s2_{};
    };
}
