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
#include "../vector/vector.hpp"

namespace zlFFT {
    template<typename FloatType>
    class WindowFunction {
    public:
        WindowFunction() = default;

        void setWindow(size_t size,
                       typename juce::dsp::WindowingFunction<FloatType>::WindowingMethod method,
                       const FloatType scale = FloatType(1),
                       const bool normalise = true, const bool cycle = true, const FloatType beta = 0) {
            if (cycle) {
                std::vector<FloatType> tempWindow;
                tempWindow.resize(size + 1);
                juce::dsp::WindowingFunction<FloatType>::fillWindowingTables(
                    tempWindow.data(), size + 1, method, normalise, beta);
                window.resize(size);
                zlVector::copy(window.data(), tempWindow.data(), size);
            } else {
                window.resize(size);
                juce::dsp::WindowingFunction<FloatType>::fillWindowingTables(
                    window.data(), size, method, normalise, beta);
            }
            window = window * scale;
        }

        void multiply(FloatType *buffer, size_t num_samples) {
            auto vector = kfr::make_univector(buffer, num_samples);
            auto window_v = kfr::make_univector(window.data(), num_samples);
            vector = vector * window_v;
        }

        void multiply(kfr::univector<FloatType> &buffer) {
            buffer = buffer * window;
        }

    private:
        kfr::univector<FloatType> window;
    };
}
