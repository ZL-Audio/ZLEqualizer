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
        preFFT.prepare(spec);
        postFFT.prepare(spec);
        sideFFT.prepare(spec);
        preBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
        postBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
        sideBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::pushPreFFTBuffer(juce::AudioBuffer<FloatType> &buffer) {
        if (isPreON.load()) {
            preBuffer.makeCopyOf(buffer, true);
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::pushPostFFTBuffer(juce::AudioBuffer<FloatType> &buffer) {
        if (isPostON.load()) {
            postBuffer.makeCopyOf(buffer, true);
        }
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
            preFFT.reset();
            postFFT.reset();
            sideFFT.reset();
        }
        if (isON.load()) {
            if (isPreON.load()) {
                preFFT.process(preBuffer);
            }
            if (isPostON.load()) {
                postFFT.process(postBuffer);
            }
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
        if (x && !isThreadRunning()) {
            startThread(juce::Thread::Priority::low);
        }
        if (!x && isThreadRunning()) {
            stopThread(-1);
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setPreON(const bool x) {
        isPreON.store(x);
        toReset.store(true);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setPostON(const bool x) {
        isPostON.store(x);
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
            const juce::Rectangle<float> bound{xx.load(), yy.load(), width.load(), height.load()};
            if (isPreON.load()) {
                preFFT.run();
                juce::ScopedLock lock(pathLock);
                preFFT.createPath(prePath, bound);
            }
            if (isPostON.load()) {
                postFFT.run();
                juce::ScopedLock lock(pathLock);
                postFFT.createPath(postPath, bound);
            }
            if (isSideON.load()) {
                sideFFT.run();
                juce::ScopedLock lock(pathLock);
                sideFFT.createPath(sidePath, bound);
            }
            isPathReady.store(true);
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::handleAsyncUpdate() {
        notify();
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setBound(juce::Rectangle<float> bound) {
        xx.store(bound.getX());
        yy.store(bound.getY());
        width.store(bound.getWidth());
        height.store(bound.getHeight());
        isBoundReady.store(false); // skip the next update cause bound is out-dated
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::updatePaths(
        juce::Path &prePath_, juce::Path &postPath_, juce::Path &sidePath_) {
        juce::ScopedLock lock(pathLock);
        if (isBoundReady.load()) {
            if (isPreON.load()) {
                prePath_ = prePath;
            }
            if (isPostON.load()) {
                postPath_ = postPath;
            }
            if (isSideON.load()) {
                sidePath_ = sidePath;
            }
        } else {
            prePath_.clear();
            postPath_.clear();
            sidePath_.clear();
            isBoundReady.store(true);
        }
        isPathReady.store(false);
    }

    template
    class PrePostFFTAnalyzer<float>;

    template
    class PrePostFFTAnalyzer<double>;
} // zlFFT
