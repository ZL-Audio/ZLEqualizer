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

#include <boost/math/interpolators/cardinal_quintic_b_spline.hpp>
#include <boost/math/interpolators/makima.hpp>

namespace zlFFT {
    template<typename FloatType>
    SingleFFTAnalyzer<FloatType>::SingleFFTAnalyzer() {
        tiltSlope.store(zlState::ffTTilt::slopes[static_cast<size_t>(zlState::ffTTilt::defaultI)]);
    }

    template<typename FloatType>
    SingleFFTAnalyzer<FloatType>::~SingleFFTAnalyzer() = default;

    template<typename FloatType>
    void SingleFFTAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        sampleRate.store(static_cast<float>(spec.sampleRate));
        int extraOrder = 0;
        if (sampleRate >= 50000) {
            extraOrder = 1;
        } else if (sampleRate >= 100000) {
            extraOrder = 2;
        }
        setOrder(extraOrder + zlState::ffTOrder::orders[static_cast<size_t>(zlState::ffTOrder::defaultI)]);
        clear();
    }

    template<typename FloatType>
    void SingleFFTAnalyzer<FloatType>::clear() {
        toClear.store(true);

    }

    template<typename FloatType>
    void SingleFFTAnalyzer<FloatType>::setOrder(int fftOrder) {
        fft = std::make_unique<juce::dsp::FFT>(fftOrder);
        window = std::make_unique<
            juce::dsp::WindowingFunction<float> >(static_cast<size_t>(fft->getSize()),
                                                  juce::dsp::WindowingFunction<float>::hann,
                                                  true);
        fftSize = static_cast<size_t>(fft->getSize());
        audioBuffer.setSize(1, static_cast<int>(fftSize.load()));
        fftBuffer.setSize(1, static_cast<int>(fftSize.load()) * 2);

        smoothedDBs.resize(fftSize.load() / 2 + 2);
        std::fill(smoothedDBs.begin(), smoothedDBs.end(), minDB * 2.f);

        deltaT.store(sampleRate.load() / static_cast<float>(fftSize.load()));
        decayRate.store(zlState::ffTSpeed::speeds[static_cast<size_t>(zlState::ffTSpeed::defaultI)]);

        smoothedDBX.resize(fftSize.load() / 2 + 2);
        smoothedDBX[0] = deltaT.load() * (-.5f);
        for (size_t idx = 1; idx < smoothedDBX.size(); ++idx) {
            smoothedDBX[idx] = smoothedDBX[idx - 1] + deltaT.load();
        }
        std::fill(interplotDBs.begin(), interplotDBs.end(), minDB * 2.f);
    }

    template<typename FloatType>
    void SingleFFTAnalyzer<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        if (toClear.load()) {
            audioIndex = 0;
            isAudioReady.store(false);
            isFFTReady.store(false);
            std::fill(smoothedDBs.begin(), smoothedDBs.end(), minDB * 2.f);
            toClear.store(false);
        }
        if (isAudioReady.load()) { return; }
        auto lBuffer = buffer.getReadPointer(0);
        auto rBuffer = buffer.getReadPointer(1);
        auto mBuffer = audioBuffer.getWritePointer(0);
        for (size_t i = 0; i < static_cast<size_t>(buffer.getNumSamples()); ++i) {
            if (audioIndex < fftSize.load()) {
                mBuffer[audioIndex] = 0.5f * static_cast<float>(lBuffer[i] + rBuffer[i]);
                audioIndex += 1;
            } else {
                isAudioReady.store(true);
                audioIndex = 0;
                break;
            }
        }
    }

    template<typename FloatType>
    void SingleFFTAnalyzer<FloatType>::run() {
        juce::ScopedNoDenormals noDenormals;
        if (isAudioReady.load() && !isFFTReady.load()) {
            juce::ScopedReadLock lock1(fftParaLock);
            fftBuffer.copyFrom(0, 0, audioBuffer, 0, 0, audioBuffer.getNumSamples());

            window->multiplyWithWindowingTable(fftBuffer.getWritePointer(0), fftSize.load());
            fft->performFrequencyOnlyForwardTransform(fftBuffer.getWritePointer(0));
            const auto mBuffer = fftBuffer.getReadPointer(0);

            const auto decay = decayRate.load();
            juce::ScopedLock lock2(ampUpdatedLock);
            for (size_t i = 0; i < fftSize.load() / 2; ++i) {
                const auto currentDB = juce::Decibels::gainToDecibels(
                    2 * mBuffer[i] / static_cast<float>(fftSize.load()), -240.f);
                smoothedDBs[i + 1] = currentDB < smoothedDBs[i + 1]
                                         ? smoothedDBs[i + 1] * decay + currentDB * (1 - decay)
                                         : currentDB;
            }
            smoothedDBs[0] = smoothedDBs[1] * 2.f;
            smoothedDBs[smoothedDBs.size() - 1] = smoothedDBs[smoothedDBs.size() - 2] * 2.f;

            std::vector<float> x = smoothedDBX;
            std::vector<float> y = smoothedDBs;

            using boost::math::interpolators::makima;
            auto spline = makima(std::move(x), std::move(y),
                                 1.f, -1.f);

            preInterplotDBs.front() = spline(static_cast<float>(zlIIR::frequencies.front()));
            preInterplotDBs.back() = spline(static_cast<float>(zlIIR::frequencies.back()));
            for (size_t i = 0; i < preInterplotDBs.size() - 2; ++i) {
                preInterplotDBs[i + 1] = spline(static_cast<float>(zlIIR::frequencies[i * preScale]));
            }

            boost::math::interpolators::cardinal_quintic_b_spline<float> spline2(
                preInterplotDBs.data(), preInterplotDBs.size(),
                -static_cast<float>(preScale), static_cast<float>(preScale),
                {0.f, 0.f}, {0.f, 0.f});

            const auto tilt = tiltSlope.load();
            for (size_t i = 0; i < interplotDBs.size(); ++i) {
                interplotDBs[i] = spline2(static_cast<float>(i * 2)) + static_cast<float>(std::log2(
                                      zlIIR::frequencies[i * 2] / 1000)) * tilt;
            }

            isAudioReady.store(false);
            isFFTReady.store(true);
        }
    }

    template<typename FloatType>
    void SingleFFTAnalyzer<FloatType>::createPath(juce::Path &path, const juce::Rectangle<float> bound) {
        juce::ScopedNoDenormals noDenormals;
        path.clear();
        juce::ScopedLock lock(ampUpdatedLock);
        path.startNewSubPath(bound.getX(), bound.getBottom() + 10.f);
        size_t i = 0;
        while (i < interplotDBs.size()) {
            const auto x = static_cast<float>(2 * i) / static_cast<float>(zlIIR::frequencies.size() - 1) *
                           bound.getWidth();
            const auto y = interplotDBs[i] / minDB * bound.getHeight() + bound.getY();
            if (i == 0) {
                path.startNewSubPath(x, y);
            } else {
                path.lineTo(x, y);
            }
            i += 1;
        }
        isFFTReady.store(false);
    }

    template
    class SingleFFTAnalyzer<float>;

    template
    class SingleFFTAnalyzer<double>;
} // zlFFT
