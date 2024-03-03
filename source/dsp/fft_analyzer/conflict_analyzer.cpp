// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "conflict_analyzer.hpp"

namespace zlFFT {
    template<typename FloatType>
    ConflictAnalyzer<FloatType>::ConflictAnalyzer(const std::string &name)
        : Thread(name){
    }

    template<typename FloatType>
    ConflictAnalyzer<FloatType>::~ConflictAnalyzer() {
        if (isThreadRunning()) {
            stopThread(-1);
        }
    }

    template<typename FloatType>
    void ConflictAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        sampleRate.store(static_cast<float>(spec.sampleRate));
        int extraOrder = 0;
        if (sampleRate >= 50000) {
            extraOrder = 1;
        } else if (sampleRate >= 100000) {
            extraOrder = 2;
        }

    }

    template<typename FloatType>
    void ConflictAnalyzer<FloatType>::pushMainBlock(juce::dsp::AudioBlock<FloatType> &block) {

    }
} // zlFFT