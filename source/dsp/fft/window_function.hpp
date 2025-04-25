// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_dsp/juce_dsp.h>
#include "kfr_import.hpp"

namespace zlFFT {
    template<typename FloatType>
    class WindowFunction {
    public:
        WindowFunction() = default;

        void setWindow(size_t size,
                       typename juce::dsp::WindowingFunction<FloatType>::WindowingMethod method,
                       bool normalise = true, FloatType beta = 0) {
            window.resize(size);
            juce::dsp::WindowingFunction<FloatType>::fillWindowingTables(
                window.data(), size, method, normalise, beta);
        }

        void multiply(FloatType *buffer, size_t num_samples) {
            auto vector = kfr::make_univector(buffer, num_samples);
            auto window_v = kfr::make_univector(window.data(), num_samples);
            vector = vector * window_v;
        }

    private:
        std::vector<FloatType> window;
    };
}
