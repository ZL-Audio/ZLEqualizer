// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "pre_post_fft_analyzer.hpp"

namespace zlFFTAnalyzer {
    template<typename FloatType>
    PrePostFFTAnalyzer<FloatType>::PrePostFFTAnalyzer(const size_t fftOrder)
        : Thread("pre_post_analyzer"),
          fftAnalyzer(fftOrder) {
        fftAnalyzer.setON({isPreON.load(), isPostON.load(), isSideON.load()});
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        fftAnalyzer.prepare(spec);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::prepareBuffer() {
        currentON = isON.load();
        if (currentON) {
            currentPreON = isPreON.load();
            currentPostON = isPostON.load();
            currentSideON = isSideON.load();
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::process(juce::AudioBuffer<FloatType> &pre,
                                                juce::AudioBuffer<FloatType> &post,
                                                juce::AudioBuffer<FloatType> &side) {
        if (toReset.exchange(false)) {
            fftAnalyzer.reset();
        }
        if (currentON) {
            fftAnalyzer.process({pre, post, side});
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setON(const bool x) {
        isON.store(x);
        if (!x) {
            triggerAsyncUpdate();
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setPreON(const bool x) {
        isPreON.store(x);
        fftAnalyzer.setON({isPreON.load(), isPostON.load(), isSideON.load()});
        toReset.store(true);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setPostON(const bool x) {
        isPostON.store(x);
        fftAnalyzer.setON({isPreON.load(), isPostON.load(), isSideON.load()});
        toReset.store(true);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setSideON(const bool x) {
        isSideON.store(x);
        fftAnalyzer.setON({isPreON.load(), isPostON.load(), isSideON.load()});
        toReset.store(true);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::run() {
        juce::ScopedNoDenormals noDenormals;
        while (!threadShouldExit()) {
            fftAnalyzer.run();
            isPathReady.store(true);
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::handleAsyncUpdate() {
        const auto x = isON.load();
        if (x && !isThreadRunning()) {
            startThread(juce::Thread::Priority::low);
        } else if (!x && isThreadRunning()) {
            stopThread(-1);
        } else if (x) {
            notify();
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::updatePaths(
        juce::Path &prePath_, juce::Path &postPath_, juce::Path &sidePath_,
        juce::Rectangle<float> bound, const float minimumFFTDB) {
        if (isPathReady.load()) {
            prePath_.clear();
            postPath_.clear();
            sidePath_.clear();
            fftAnalyzer.createPath({prePath_, postPath_, sidePath_}, bound, minimumFFTDB);
            isPathReady.store(false);
        }
        triggerAsyncUpdate();
    }

    template
    class PrePostFFTAnalyzer<float>;

    template
    class PrePostFFTAnalyzer<double>;
} // zlFFT
