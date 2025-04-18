// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../fft_analyzer/fft_analyzer.hpp"

namespace zlEqMatch {
    template<typename FloatType>
    class EqMatchAnalyzer final : private juce::Thread {
    public:
        enum MatchMode {
            matchSide,
            matchPreset,
            matchSlope
        };

        static constexpr size_t pointNum = 251, smoothSize = 11;
        static constexpr float avgDB = -36.f;

        explicit EqMatchAnalyzer(size_t fftOrder = 12);

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<FloatType> &bBuffer, juce::AudioBuffer<FloatType> &tBuffer);

        zlFFT::AverageFFTAnalyzer<FloatType, 2, pointNum> &getAverageFFT() { return fftAnalyzer; }

        void setON(bool x);

        void reset();

        void checkRun();

        void setMatchMode(MatchMode mode) {
            mMode.store(mode);
        }

        void setTargetSlope(float x);

        void setTargetPreset(const std::array<float, pointNum> &dBs);

        void updatePaths(juce::Path &mainP, juce::Path &targetP, juce::Path &diffP,
                         juce::Rectangle<float> bound, std::array<float, 3> minDBs);

        void setSmooth(const float x) {
            smooth.store(x);
            toUpdateSmooth.store(true);
        }

        void setSlope(const float x) { slope.store(x); }

        void setShift(const float x) { shift.store(x); }

        std::array<std::atomic<float>, pointNum> &getTarget() { return atomicTargetDBs; }

        std::array<std::atomic<float>, pointNum> &getDiffs() { return atomicDiffs; }

        void setDrawingDiffs(const size_t idx, const float x) {
            drawingDiffs[idx].store(x - shift.load());
            drawingFlag[idx].store(true);
        }

        void clearDrawingDiffs(const size_t idx) {
            drawingFlag[idx].store(false);
        }

        void clearDrawingDiffs() {
            for (size_t idx = 0; idx < pointNum; ++idx) {
                drawingFlag[idx].store(false);
            }
        }

    private:
        zlFFT::AverageFFTAnalyzer<FloatType, 2, pointNum> fftAnalyzer;
        std::array<float, pointNum> mainDBs{}, targetDBs{}, diffs{};
        std::array<std::atomic<float>, pointNum> atomicTargetDBs{}, atomicDiffs{};
        std::array<std::atomic<bool>, pointNum> drawingFlag;
        std::array<std::atomic<float>, pointNum> drawingDiffs;
        std::atomic<MatchMode> mMode;
        std::atomic<bool> isON{false};
        std::array<float, pointNum> loadDBs{};
        std::atomic<bool> toUpdateFromLoadDBs{false};

        std::atomic<float> smooth{.5f}, slope{.0f}, shift{0.f};
        std::atomic<bool> toUpdateSmooth{true};
        std::array<float, smoothSize> smoothKernel{};
        float rescale = 1.f;
        std::array<float, pointNum + smoothSize - 1> originalDiffs{};

        void run() override;

        void updateSmooth() {
            if (toUpdateSmooth.exchange(false)) {
                smoothKernel[smoothSize / 2] = 1.0;
                const auto currentSmooth = std::clamp(smooth.load(), 0.f, .5f);
                rescale = std::clamp(2.f - 2 * smooth.load(), 0.f, 1.f);
                constexpr float midSlope = -1.f / static_cast<float>(smoothSize / 2);
                const auto tempSlope = currentSmooth < 0.5
                                           ? -1.f * (1 - currentSmooth * 2.f) + currentSmooth * 2.f * midSlope
                                           : (2.f - 2.f * currentSmooth) * midSlope;
                for (size_t i = 1; i < smoothSize / 2 + 1; i++) {
                    smoothKernel[smoothSize / 2 + i] = std::max(tempSlope * static_cast<float>(i) + 1, 0.f);
                    smoothKernel[smoothSize / 2 - i] = smoothKernel[smoothSize / 2 + i];
                }
                const auto kernelC = 1.f /
                                     std::max(std::reduce(smoothKernel.begin(), smoothKernel.end(), 0.0f), 0.01f);
                for (auto &x: smoothKernel) {
                    x *= kernelC;
                }
            }
        }
    };
} // zlEqMatch
