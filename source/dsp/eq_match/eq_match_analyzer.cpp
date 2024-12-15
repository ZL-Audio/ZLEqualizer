// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "eq_match_analyzer.hpp"

namespace zlEqMatch {
    template<typename FloatType>
    EqMatchAnalyzer<FloatType>::EqMatchAnalyzer(const size_t fftOrder)
        : Thread("eq_match_analyzer"), fftAnalyzer(fftOrder) {
        std::fill(mainDBs.begin(), mainDBs.end(), avgDB);
        std::fill(targetDBs.begin(), targetDBs.end(), avgDB);
        std::fill(diffs.begin(), diffs.end(), avgDB);
        updateSmooth();
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        fftAnalyzer.prepare(spec);
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::process(juce::AudioBuffer<FloatType> &bBuffer,
                                             juce::AudioBuffer<FloatType> &tBuffer) {
        if (isON.load()) {
            if (toReset.exchange(false)) {
                fftAnalyzer.reset();
            }
            fftAnalyzer.process({bBuffer, tBuffer});
        }
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::setON(const bool x) {
        if (x != isON.load()) {
            if (x) {
                fftAnalyzer.setON(0, true);
                fftAnalyzer.setON(1, true);
                isON.store(true);
                if (!isThreadRunning()) {
                    startThread();
                }
            } else {
                isON.store(false);
                fftAnalyzer.setON(0, false);
                fftAnalyzer.setON(1, false);
                if (isThreadRunning()) {
                    stopThread(-1);
                }
            }
        }
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::run() {
        juce::ScopedNoDenormals noDenormals;
        while (!threadShouldExit()) {
            fftAnalyzer.run();
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
        }
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::checkRun() {
        notify();
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::updatePaths(juce::Path &mainP, juce::Path &targetP, juce::Path &diffP,
                                                 const juce::Rectangle<float> bound) {
        // update mainDBs and targetDBs
        mainDBs = fftAnalyzer.getInterplotDBs(0);
        for (auto &dB : mainDBs) {
            dB += 10.f;
        }
        if (mMode.load() == MatchMode::matchSide) {
            targetDBs = fftAnalyzer.getInterplotDBs(1);
            for (auto &dB : targetDBs) {
                dB += 10.f;
            }
        } else if (toUpdateFromLoadDBs.load()) {
            targetDBs = loadDBs;
            toUpdateFromLoadDBs.store(false);
        }
        // calculate original diffs
        for (size_t i = 0; i < pointNum; ++i) {
            originalDiffs[i + smoothKernel.size() / 2] = targetDBs[i] - mainDBs[i];
        }
        // pad original diffs
        for (size_t i = 0; i < smoothKernel.size() / 2; ++i) {
            originalDiffs[i] = originalDiffs[smoothKernel.size() / 2];
        }
        for (size_t i = pointNum + smoothKernel.size() / 2; i < originalDiffs.size(); ++i) {
            originalDiffs[i] = originalDiffs[pointNum + smoothKernel.size() / 2 - 1];
        }
        // update smooth and slope
        updateSmooth();
        // apply smooth
        std::fill(diffs.begin(), diffs.end(), 0.f);
        for (size_t i = 0; i < pointNum; ++i) {
            for (size_t j = 0; j < smoothKernel.size(); ++j) {
                diffs[i] += originalDiffs[i + j] * smoothKernel[j];
            }
        }
        // apply slope
        const auto slopeTotal = (fftAnalyzer.maxFreqLog2 - fftAnalyzer.minFreqLog2) * slope.load();
        const float slopeDelta = slopeTotal / static_cast<float>(pointNum - 1);
        float slopeShift = -slopeTotal * .5f;
        for (size_t i = 0; i < pointNum; ++i) {
            diffs[i] += slopeShift;
            slopeShift += slopeDelta;
        }
        // center diffs
        const auto diffC = std::reduce(diffs.begin(), diffs.end(), 0.f) / static_cast<float>(diffs.size()) - avgDB;
        for (auto &diff: diffs) {
            diff -= diffC;
        }
        // create paths
        const float width = bound.getWidth(), height = bound.getHeight(), boundY = bound.getY();
        const std::array<juce::Path *, 3> paths{&mainP, &targetP, &diffP};
        for (const auto &p: paths) {
            p->clear();
        }
        const std::array<std::array<float, pointNum> *, 3> dBs{&mainDBs, &targetDBs, &diffs};
        for (size_t idx = 0; idx < pointNum; ++idx) {
            const auto x = static_cast<float>(idx) / static_cast<float>(pointNum - 1) * width;
            for (size_t i = 0; i < 3; ++i) {
                const auto y = dBs[i]->at(idx) / fftAnalyzer.minDB * height + boundY;
                paths[i]->lineTo(x, y);
            }
        }
    }

    template
    class EqMatchAnalyzer<float>;

    template
    class EqMatchAnalyzer<double>;
} // zlEqMatch
