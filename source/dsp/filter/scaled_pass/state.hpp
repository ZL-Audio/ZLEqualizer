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
#include "normalized.hpp"

namespace zldsp::filter::scaled_pass {
    class State {
    public:
        void prepare(const size_t channels) {
            states_.assign(channels * kMaxStates, 0.0);
        }

        void reset() {
            std::ranges::fill(states_, 0.0);
        }

        double process(const NormalizedCoefficients& coefficients, const size_t channel, double sample) {
            auto* states = states_.data() + channel * kMaxStates;
            auto output = coefficients.taps[0] * sample;
            size_t k{};
            for (size_t i = 0; i < coefficients.first_count; ++i) {
                const auto& rotation = coefficients.first[i];
                output += coefficients.taps[k + 1] * states[k];
                const auto next = rotation.sine * states[k] - rotation.cosine * sample;
                states[k] = rotation.sine * sample + rotation.cosine * states[k];
                sample = next;
                ++k;
            }
            for (size_t i = 0; i < coefficients.second_count; ++i) {
                const auto& b = coefficients.second[i][0];
                const auto& c = coefficients.second[i][1];
                output += coefficients.taps[k + 1] * states[k] + coefficients.taps[k + 2] * states[k + 1];
                const auto v = c.sine * sample + c.cosine * states[k];
                sample = c.sine * states[k] - c.cosine * sample;
                states[k] = b.sine * states[k + 1] - b.cosine * v;
                states[k + 1] = b.sine * v + b.cosine * states[k + 1];
                k += 2;
            }
            return output;
        }

    private:
        std::vector<double> states_{};
    };
}
