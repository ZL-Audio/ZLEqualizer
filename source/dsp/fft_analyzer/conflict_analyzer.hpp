// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_CONFLICT_ANALYZER_HPP
#define ZLEqualizer_CONFLICT_ANALYZER_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlFFT {
    template<typename FloatType>
    class ConflictAnalyzer final : private juce::Thread {
    public:
        explicit ConflictAnalyzer(const std::string &name);

        ~ConflictAnalyzer() override;

        void prepare(const juce::dsp::ProcessSpec &spec);

        void clear();

        void pushMainBlock(juce::dsp::AudioBlock<FloatType> &block);

        void pushRefBlock(juce::dsp::AudioBlock<FloatType> &block);

    private:
        static constexpr auto minFreq = 20.f, maxFreq = 22000.f, minDB = -72.f;
        std::atomic<float> sampleRate;
        juce::ReadWriteLock fftParaLock;

        std::atomic<FloatType> strength;
        juce::AudioBuffer<FloatType> mainBuffer, refBuffer;

        void run() override;
    };
} // zlFFT

#endif //ZLEqualizer_CONFLICT_ANALYZER_HPP
