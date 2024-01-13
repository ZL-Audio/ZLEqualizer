// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fft_analyzer.hpp"

namespace zlFFT {
    template<typename FloatType>
    FFTAnalyzer<FloatType>::FFTAnalyzer(const std::string &name) : Thread(name) {
        startThread(juce::Thread::Priority::low);
    }

    template<typename FloatType>
    FFTAnalyzer<FloatType>::~FFTAnalyzer() {
        stopThread(-1);
    }

    template<typename FloatType>
    void FFTAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        sampleRate.store(static_cast<float>(spec.sampleRate));
        setOrder(12);
    }

    template<typename FloatType>
    void FFTAnalyzer<FloatType>::setOrder(int fftOrder) {
        juce::ScopedWriteLock lock(fftParaLock);
        fft = std::make_unique<juce::dsp::FFT>(fftOrder);
        window = std::make_unique<
            juce::dsp::WindowingFunction<float> >(static_cast<size_t>(fft->getSize()),
                                                  juce::dsp::WindowingFunction<float>::hann,
                                                  true);
        audioBuffer.setSize(1, fft->getSize() * 2);
        // fftAmplitudes.resize(fft->getSize());
        smoothedDBs.resize(static_cast<size_t>(fft->getSize()) * 2);
        for (auto &s: smoothedDBs) {
            s.reset(30);
        }
    }

    template<typename FloatType>
    void FFTAnalyzer<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        if (isAudioReady.load()) { return; }
        auto lBuffer = buffer.getReadPointer(0);
        auto rBuffer = buffer.getReadPointer(1);
        auto mBuffer = audioBuffer.getWritePointer(0);
        juce::ScopedReadLock lock(fftParaLock);
        for (size_t i = 0; i < static_cast<size_t>(buffer.getNumSamples()); ++i) {
            if (audioIndex < static_cast<size_t>(audioBuffer.getNumSamples())) {
                mBuffer[audioIndex] = 0.5f * static_cast<float>(lBuffer[i] + rBuffer[i]);
                audioIndex += 1;
            } else {
                isAudioReady.store(true);
                audioIndex = 0;
                notify();
                break;
            }
        }
    }

    template<typename FloatType>
    void FFTAnalyzer<FloatType>::run() {
        while (!threadShouldExit()) {
            if (isAudioReady.load()) {
                juce::ScopedReadLock lock1(fftParaLock);

                window->multiplyWithWindowingTable(audioBuffer.getWritePointer(0), static_cast<size_t>(audioBuffer.getNumSamples()));
                fft->performFrequencyOnlyForwardTransform(audioBuffer.getWritePointer(0));
                const auto mBuffer = audioBuffer.getReadPointer(0);

                juce::ScopedLock lock2(ampUpdatedLock);
                for (size_t i = 0; i < static_cast<size_t>(audioBuffer.getNumSamples()); ++i) {
                    smoothedDBs[i].setTargetValue(juce::Decibels::gainToDecibels(juce::jmin(0.f, mBuffer[i])));
                }

                isAudioReady.store(false);
            } else {
                const auto flag = wait(100);
                juce::ignoreUnused(flag);
            }
        }
    }

    template<typename FloatType>
    void FFTAnalyzer<FloatType>::createPath(juce::Path &path, const juce::Rectangle<float> bounds) {
        path.clear();
        juce::ScopedLock lock(ampUpdatedLock);
        path.startNewSubPath(indexToX(0, bounds), binToY(smoothedDBs[0].getNextValue(), bounds));
        for (size_t i = 1; i < static_cast<size_t>(fft->getSize()); ++i) {
            path.lineTo(indexToX(i, bounds), binToY(smoothedDBs[i].getNextValue(), bounds));
        }
    }

    template
    class FFTAnalyzer<float>;

    template
    class FFTAnalyzer<double>;
} // zlFFT
