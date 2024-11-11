// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "eq_match_analyzer.hpp"

namespace zlEqMatch {
    template<typename FloatType>
    EqMatchAnalyzer<FloatType>::EqMatchAnalyzer(size_t fftOrder)
        : Thread("eq_match_analyzer"), fftAnalyzer(fftOrder){
        fftAnalyzer.setON(isON.load());
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        fftAnalyzer.prepare(spec);
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::process(juce::AudioBuffer<FloatType> &bBuffer,
        juce::AudioBuffer<FloatType> &tBuffer) {
        if (toReset.exchange(false)) {
            fftAnalyzer.reset();
        }
        if (isON.load()) {
            fftAnalyzer.process({bBuffer, tBuffer});
            if (fftAnalyzer.getReadyForNextFFT()) {
                triggerAsyncUpdate();
            }
        }
    }

    template<typename FloatType>
    void EqMatchAnalyzer<FloatType>::setON(const bool x) {
        isON.store(x);
        if (!x) {
            triggerAsyncUpdate();
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
    void EqMatchAnalyzer<FloatType>::handleAsyncUpdate() {
        const auto x = isON.load();
        if (x && !isThreadRunning()) {
            startThread(juce::Thread::Priority::low);
        } else if (!x && isThreadRunning()) {
            stopThread(-1);
        } else if (x) {
            notify();
        }
    }

    template
    class EqMatchAnalyzer<float>;

    template
    class EqMatchAnalyzer<double>;
} // zlEqMatch