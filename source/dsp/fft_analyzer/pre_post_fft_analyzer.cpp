// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "pre_post_fft_analyzer.hpp"

namespace zlFFT {
    template<typename FloatType>
    PrePostFFTAnalyzer<FloatType>::PrePostFFTAnalyzer()
        : Thread("pre_post_analyzer") {
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        syncFFT.prepare(spec);
        sideFFT.prepare(spec);
        preBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
        postBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
        sideBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::pushPreFFTBuffer(juce::AudioBuffer<FloatType> &buffer) {
        preBuffer.makeCopyOf(buffer, true);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::pushPostFFTBuffer(juce::AudioBuffer<FloatType> &buffer) {
        postBuffer.makeCopyOf(buffer, true);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::pushSideFFTBuffer(juce::AudioBuffer<FloatType> &buffer) {
        if (isSideON.load()) {
            sideBuffer.makeCopyOf(buffer, true);
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::process() {
        if (toReset.exchange(false)) {
            syncFFT.reset();
            sideFFT.reset();
        }
        if (isON.load()) {
            syncFFT.process(preBuffer, postBuffer);
            if (isSideON.load()) {
                sideFFT.process(sideBuffer);
            }
            if (!isPathReady.load()) {
                triggerAsyncUpdate();
            }
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setON(const bool x) {
        isON.store(x);
        triggerAsyncUpdate();
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setPreON(const bool x) {
        isPreON.store(x);
        syncFFT.setON(x, isPostON.load());
        toReset.store(true);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setPostON(const bool x) {
        isPostON.store(x);
        syncFFT.setON(isPreON.load(), x);
        toReset.store(true);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setSideON(const bool x) {
        isSideON.store(x);
        toReset.store(true);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::run() {
        juce::ScopedNoDenormals noDenormals;
        while (!threadShouldExit()) {
            syncFFT.run();
            if (isSideON.load()) {
                sideFFT.run();
            }
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
        juce::Path &prePath_, juce::Path &postPath_, juce::Path &sidePath_, juce::Rectangle<float> bound) {
        prePath_.clear();
        postPath_.clear();
        sidePath_.clear();
        if (isPreON.load() || isPostON.load()) {
            syncFFT.createPath(prePath_, postPath_, bound);
        }
        if (isSideON.load()) {
            sideFFT.createPath(sidePath_, bound);
        }
        isPathReady.store(false);
    }

    template
    class PrePostFFTAnalyzer<float>;

    template
    class PrePostFFTAnalyzer<double>;
} // zlFFT
