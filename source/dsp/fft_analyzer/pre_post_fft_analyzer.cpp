// Copyright (C) 2023 - zsliu98
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
        if (isON.load()) {
            const auto f1 = !preFFT.getIsAudioReady() || !isPreON.load();
            const auto f2 = !postFFT.getIsAudioReady() || !isPostON.load();
            const auto f3 = !sideFFT.getIsAudioReady() || !isSideON.load();
            if (f1 && f2 && f3) {
                preFFT.process(preBuffer);
                postFFT.process(postBuffer);
                sideFFT.process(sideBuffer);
            } else {
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
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setPostON(const bool x) {
        isPostON.store(x);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setSideON(const bool x) {
        isSideON.store(x);
    }

    template<typename FloatType>
    bool PrePostFFTAnalyzer<FloatType>::isFFTReady() {
        if (isPreON.load() && !preFFT.getIsFFTReady()) {
            return false;
        }
        if (isPostON.load() && !postFFT.getIsFFTReady()) {
            return false;
        }
        if (isSideON.load() && !sideFFT.getIsFFTReady()) {
            return false;
        }
        return true;
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::run() {
        juce::ScopedNoDenormals noDenormals;
        while (!threadShouldExit()) {
            if (isPreON.load() && preFFT.getIsAudioReady()) {
                preFFT.run();
            }
            if (isPostON.load() && postFFT.getIsAudioReady()) {
                postFFT.run();
            }
            if (isSideON.load() && sideFFT.getIsAudioReady()) {
                sideFFT.run();
            }
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::handleAsyncUpdate() {
        notify();
    }

    template
    class PrePostFFTAnalyzer<float>;

    template
    class PrePostFFTAnalyzer<double>;
} // zlFFT