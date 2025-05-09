// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "eq_match_analyzer.hpp"

namespace zldsp::eq_match {
    template<typename FloatType>
    EqMatchAnalyzer<FloatType>::EqMatchAnalyzer(const size_t fftOrder)
        : Thread("eq_match_analyzer"), fftAnalyzer(fftOrder) {
        std::fill(mainDBs.begin(), mainDBs.end(), 0.f);
        std::fill(targetDBs.begin(), targetDBs.end(), 0.f);
        std::fill(diffs.begin(), diffs.end(), 0.f);
        updateSmooth();
        clearDrawingDiffs();
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        fftAnalyzer.prepare(spec);
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::process(juce::AudioBuffer<FloatType> &bBuffer,
                                             juce::AudioBuffer<FloatType> &tBuffer) {
        if (isON.load()) {
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
                    startThread(Priority::low);
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
    void EqMatchAnalyzer<FloatType>::reset() {
        fftAnalyzer.reset();
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
    void EqMatchAnalyzer<FloatType>::setTargetSlope(const float x) {
        const float tiltShiftTotal = (fftAnalyzer.maxFreqLog2 - fftAnalyzer.minFreqLog2) * x;
        const float tiltShiftDelta = tiltShiftTotal / static_cast<float>(pointNum - 1);
        float tiltShift = -tiltShiftTotal * .5f;
        if (toUpdateFromLoadDBs.load() == false) {
            for (size_t i = 0; i < loadDBs.size(); i++) {
                loadDBs[i] = tiltShift;
                tiltShift += tiltShiftDelta;
            }
            toUpdateFromLoadDBs.store(true);
        }
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::setTargetPreset(const std::array<float, pointNum> &dBs) {
        if (toUpdateFromLoadDBs.load() == false) {
            for (size_t i = 0; i < dBs.size(); i++) {
                loadDBs[i] = dBs[i];
            }
            toUpdateFromLoadDBs.store(true);
        }
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::updatePaths(juce::Path &mainP, juce::Path &targetP, juce::Path &diffP,
                                                 const juce::Rectangle<float> bound,
                                                 const std::array<float, 3> minDBs) {
        // update mainDBs and targetDBs
        {
            mainDBs = fftAnalyzer.getInterplotDBs(0);
            const auto maxDB = *std::max_element(mainDBs.begin(), mainDBs.end());
            for (auto &dB: mainDBs) {
                dB -= maxDB;
            }
        }
        if (mMode.load() == MatchMode::matchSide) {
            targetDBs = fftAnalyzer.getInterplotDBs(1);
            const auto maxDB = *std::max_element(targetDBs.begin(), targetDBs.end());
            for (auto &dB: targetDBs) {
                dB -= maxDB;
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
        // scale diffs
        if (rescale < .99f) {
            for (auto &diff: diffs) {
                diff *= rescale;
            }
        }
        // center diffs
        const auto currentShift = shift.load();
        const auto diffC = std::reduce(
            diffs.begin(), diffs.end(), 0.f) / static_cast<float>(diffs.size()) - currentShift;
        for (size_t i = 0; i < pointNum; ++i) {
            diffs[i] = drawingFlag[i].load() ? drawingDiffs[i].load() + currentShift : diffs[i] - diffC;
        }
        // save to target
        for (size_t i = 0; i < pointNum; ++i) {
            atomicTargetDBs[i].store(targetDBs[i]);
            atomicDiffs[i].store(diffs[i]);
        }
        // create paths
        const float width = bound.getWidth(), height = bound.getHeight(), boundY = bound.getY();
        const std::array<juce::Path *, 3> paths{&mainP, &targetP, &diffP};
        for (const auto &p: paths) {
            p->clear();
        }
        const std::array<std::array<float, pointNum> *, 3> dBs{&mainDBs, &targetDBs, &diffs};
        for (size_t i = 0; i < 3; ++i) {
            const auto y = (dBs[i]->at(0) / minDBs[i] + .5f) * height + boundY;
            paths[i]->startNewSubPath(0.f, y);
        }
        for (size_t idx = 1; idx < pointNum; ++idx) {
            const auto x = static_cast<float>(idx) / static_cast<float>(pointNum - 1) * width;
            for (size_t i = 0; i < 3; ++i) {
                const auto y = (dBs[i]->at(idx) / minDBs[i] + .5f) * height + boundY;
                paths[i]->lineTo(x, y);
            }
        }
    }

    template
    class EqMatchAnalyzer<float>;

    template
    class EqMatchAnalyzer<double>;
} // zldsp::eq_match
