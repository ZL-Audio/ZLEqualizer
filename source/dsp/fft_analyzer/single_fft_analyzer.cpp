// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "single_fft_analyzer.hpp"

#include "../../state/state_definitions.hpp"

namespace zlFFT {
    template<typename FloatType>
    SingleFFTAnalyzer<FloatType>::SingleFFTAnalyzer(const std::string &name) : Thread(name) {
        startThread(juce::Thread::Priority::low);
        tiltSlope.store(zlState::ffTTilt::slopes[static_cast<size_t>(zlState::ffTTilt::defaultI)]);
    }

    template<typename FloatType>
    SingleFFTAnalyzer<FloatType>::~SingleFFTAnalyzer() {
        if (isThreadRunning()) {
            stopThread(-1);
        }
    }

    template<typename FloatType>
    void SingleFFTAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        sampleRate.store(static_cast<float>(spec.sampleRate));
        setOrder(zlState::ffTOrder::orders[static_cast<size_t>(zlState::ffTOrder::defaultI)]);
    }

    template<typename FloatType>
    void SingleFFTAnalyzer<FloatType>::setOrder(int fftOrder) {
        juce::ScopedWriteLock lock(fftParaLock);
        fft = std::make_unique<juce::dsp::FFT>(fftOrder);
        window = std::make_unique<
            juce::dsp::WindowingFunction<float> >(static_cast<size_t>(fft->getSize()),
                                                  juce::dsp::WindowingFunction<float>::hann,
                                                  true);
        fftSize = static_cast<size_t>(fft->getSize());
        audioBuffer.setSize(1, static_cast<int>(fftSize.load()));
        fftBuffer.setSize(1, static_cast<int>(fftSize.load()) * 2);

        smoothedDBs.resize(fftSize.load() / 2 + 2);
        std::fill(smoothedDBs.begin(), smoothedDBs.end(), minDB);

        deltaT.store(sampleRate.load() / static_cast<float>(fftSize.load()));
        decayRate.store(zlState::ffTSpeed::speeds[static_cast<size_t>(zlState::ffTSpeed::defaultI)]);
    }

    template<typename FloatType>
    void SingleFFTAnalyzer<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        if (isAudioReady.load()) { return; }
        auto lBuffer = buffer.getReadPointer(0);
        auto rBuffer = buffer.getReadPointer(1);
        auto mBuffer = audioBuffer.getWritePointer(0);
        juce::ScopedReadLock lock(fftParaLock);
        for (size_t i = 0; i < static_cast<size_t>(buffer.getNumSamples()); ++i) {
            if (audioIndex < fftSize.load()) {
                mBuffer[audioIndex] = 0.5f * static_cast<float>(lBuffer[i] + rBuffer[i]);
                audioIndex += 1;
            } else {
                isAudioReady.store(true);
                audioIndex = 0;
                if (!isFFTReady.load()) {
                    notify();
                }
                break;
            }
        }
    }

    template<typename FloatType>
    void SingleFFTAnalyzer<FloatType>::run() {
        while (!threadShouldExit()) {
            if (isAudioReady.load() && !isFFTReady.load()) {
                juce::ScopedReadLock lock1(fftParaLock);
                fftBuffer.copyFrom(0, 0, audioBuffer, 0, 0, audioBuffer.getNumSamples());

                window->multiplyWithWindowingTable(fftBuffer.getWritePointer(0), fftSize.load());
                fft->performFrequencyOnlyForwardTransform(fftBuffer.getWritePointer(0));
                const auto mBuffer = fftBuffer.getReadPointer(0);

                const auto decay = currentDecay.load();
                juce::ScopedLock lock2(ampUpdatedLock);
                for (size_t i = 0; i < fftSize.load() / 2; ++i) {
                    const auto currentDB = juce::Decibels::gainToDecibels(
                        2 * mBuffer[i] / static_cast<float>(fftSize.load()));
                    smoothedDBs[i + 1] = currentDB < smoothedDBs[i + 1]
                                             ? smoothedDBs[i + 1] * decay + currentDB * (1 - decay)
                                             : currentDB;
                }

                boost::math::interpolators::cardinal_cubic_b_spline<float> spline(
                    smoothedDBs.data(), smoothedDBs.size(),
                    -0.5f * deltaT.load(), deltaT.load());

                preInterplotDBs.front() = spline(static_cast<float>(zlIIR::frequencies.front()));
                preInterplotDBs.back() = spline(static_cast<float>(zlIIR::frequencies.back()));
                for (size_t i = 0; i < preInterplotDBs.size() - 2; ++i) {
                    preInterplotDBs[i + 1] = spline(static_cast<float>(zlIIR::frequencies[i * preScale]));
                }

                boost::math::interpolators::cardinal_cubic_b_spline<float> spline2(
                    preInterplotDBs.data(), preInterplotDBs.size(),
                    -static_cast<float>(preScale), static_cast<float>(preScale));

                const auto tilt = tiltSlope.load();
                for (size_t i = 0; i < interplotDBs.size(); ++i) {
                    interplotDBs[i] = spline2(static_cast<float>(i)) + static_cast<float>(std::log2(zlIIR::frequencies[i] / 1000)) * tilt;
                }

                isAudioReady.store(false);
                isFFTReady.store(true);
            } else {
                const auto flag = wait(-1);
                juce::ignoreUnused(flag);
            }
        }
    }

    template<typename FloatType>
    void SingleFFTAnalyzer<FloatType>::createPath(juce::Path &path, const juce::Rectangle<float> bound) {
        path.clear();
        juce::ScopedLock lock(ampUpdatedLock);
        path.startNewSubPath(bound.getX(), bound.getBottom() + 10.f);
        size_t i = 0;
        while (i < zlIIR::frequencies.size()) {
            const auto x = static_cast<float>(i) / static_cast<float>(zlIIR::frequencies.size() - 1) * bound.
                           getWidth();
            const auto y = interplotDBs[i] / minDB * bound.getHeight() + bound.getY();
            if (i == 0) {
                path.startNewSubPath(x, y);
            } else {
                path.lineTo(x, y);
            }
            i += 1;
        }
        isFFTReady.store(false);
        notify();
    }

    template
    class SingleFFTAnalyzer<float>;

    template
    class SingleFFTAnalyzer<double>;
} // zlFFT
