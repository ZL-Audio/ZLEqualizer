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

#include "single_fft_analyzer.hpp"

namespace zlFFT {
    template<typename FloatType>
    class ConflictAnalyzer final : private juce::Thread {
    public:
        explicit ConflictAnalyzer();

        ~ConflictAnalyzer() override;

        void prepare(const juce::dsp::ProcessSpec &spec);

        void setON(bool x);

        bool getON() const { return isON.load(); }

        void setStrength(const FloatType x) { strength.store(x); }

        void setConflictScale(const FloatType x) { conflictScale.store(x); }

        void pushMainBuffer(juce::AudioBuffer<FloatType> &buffer);

        void pushRefBuffer(juce::AudioBuffer<FloatType> &buffer);

        void process();

        bool getIsConflictReady() const { return isConflictReady.load(); }

        // void createPath(juce::Path &path, juce::Rectangle<float> bound);

        void drawRectangles(juce::Graphics &g, juce::Colour colour, juce::Rectangle<float> bound);

    private:
        SingleFFTAnalyzer<FloatType> mainAnalyzer, refAnalyzer;
        juce::AudioBuffer<FloatType> mainBuffer, refBuffer;
        std::atomic<FloatType> strength{.375f}, conflictScale{1.f};
        std::atomic<bool> isON{false}, isConflictReady{false};

        // std::array<float, zlIIR::frequencies.size() / 8> conflictsActual{};
        std::array<float, zlIIR::frequencies.size() / 8> conflicts{};
        std::vector<std::pair<float, float> > conflictAreas;
        juce::CriticalSection areaLock;

        void run() override;
    };
} // zlFFT

#endif //ZLEqualizer_CONFLICT_ANALYZER_HPP
