// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "sync_fft_analyzer.hpp"

#include "../../state/state_definitions.hpp"

#include <boost/math/interpolators/cardinal_quintic_b_spline.hpp>
#include <boost/math/interpolators/makima.hpp>

namespace zlFFT {
    template<typename FloatType>
    SyncFFTAnalyzer<FloatType>::SyncFFTAnalyzer() {
        tiltSlope.store(zlState::ffTTilt::slopes[static_cast<size_t>(zlState::ffTTilt::defaultI)]);
    }

    template<typename FloatType>
    SyncFFTAnalyzer<FloatType>::~SyncFFTAnalyzer() = default;

    template<typename FloatType>
    void SyncFFTAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        sampleRate.store(static_cast<float>(spec.sampleRate));
        int extraOrder = 0;
        if (sampleRate >= 50000) {
            extraOrder = 1;
        } else if (sampleRate >= 100000) {
            extraOrder = 2;
        }
        setOrder(extraOrder + zlState::ffTOrder::orders[static_cast<size_t>(zlState::ffTOrder::defaultI)]);
        reset();
        isPrepared.store(true);
    }

    template<typename FloatType>
    void SyncFFTAnalyzer<FloatType>::reset() {
        for (size_t i = 0; i < interplotDBs[0].size(); ++i) {
            interplotDBs[0][i].store(minDB * 2.f);
            interplotDBs[1][i].store(minDB * 2.f);
        }
        toClear.store(true);
    }

    template<typename FloatType>
    void SyncFFTAnalyzer<FloatType>::setOrder(int fftOrder) {
        fft = std::make_unique<juce::dsp::FFT>(fftOrder);
        window = std::make_unique<
            juce::dsp::WindowingFunction<float> >(static_cast<size_t>(fft->getSize()),
                                                  juce::dsp::WindowingFunction<float>::hann,
                                                  true);
        fftSize.store(static_cast<size_t>(fft->getSize()));
        deltaT.store(sampleRate.load() / static_cast<float>(fftSize.load()));
        decayRate.store(zlState::ffTSpeed::speeds[static_cast<size_t>(zlState::ffTSpeed::defaultI)]);
        fftBuffer.setSize(1, static_cast<int>(fftSize.load()) * 2);

        currentPos = 0;
        for (size_t z = 0; z < 2; ++z) {
            currentBuffer[z].resize(fftSize.load());
            std::fill(currentBuffer[z].begin(), currentBuffer[z].end(), 0.f);
            audioBuffer[z][0].setSize(1, static_cast<int>(fftSize.load()));
            audioBuffer[z][1].setSize(1, static_cast<int>(fftSize.load()));

            smoothedDBs[z].resize(fftSize.load() / 2 + 2);
            std::fill(smoothedDBs[z].begin(), smoothedDBs[z].end(), minDB * 2.f);

            for (size_t i = 0; i < interplotDBs[z].size(); ++i) {
                interplotDBs[z][i].store(minDB * 2.f);
            }
        }
        smoothedDBX.resize(fftSize.load() / 2 + 2);
        smoothedDBX[0] = deltaT.load() * (-.5f);
        for (size_t idx = 1; idx < smoothedDBX.size(); ++idx) {
            smoothedDBX[idx] = smoothedDBX[idx - 1] + deltaT.load();
        }
    }

    template<typename FloatType>
    void SyncFFTAnalyzer<FloatType>::process(juce::AudioBuffer<FloatType> &buffer1,
                                             juce::AudioBuffer<FloatType> &buffer2) {
        // if to clear, reset circular idx to zero
        if (toClear.exchange(false)) {
            currentPos = 0;
            std::fill(currentBuffer[0].begin(), currentBuffer[0].end(), 0.f);
            std::fill(currentBuffer[1].begin(), currentBuffer[1].end(), 0.f);
        }
        // write to the circular buffer
        auto lBuffer1 = buffer1.getReadPointer(0);
        auto rBuffer1 = buffer1.getReadPointer(1);
        auto lBuffer2 = buffer2.getReadPointer(0);
        auto rBuffer2 = buffer2.getReadPointer(1);
        const auto bufferSize = std::min(buffer1.getNumSamples(), buffer2.getNumSamples());
        for (size_t i = 0; i < static_cast<size_t>(bufferSize); ++i) {
            currentBuffer[0][currentPos] = 0.5f * static_cast<float>(lBuffer1[i] + rBuffer1[i]);
            currentBuffer[1][currentPos] = 0.5f * static_cast<float>(lBuffer2[i] + rBuffer2[i]);
            currentPos = (currentPos + 1) % currentBuffer[0].size();
        }
        // write to the double buffer
        const auto dIdx = static_cast<size_t>(doubleBufferIdx.fetch_or(BIT_BUSY) & BIT_IDX);
        for (size_t z = 0; z < 2; ++z) {
            if (!isON[z].load()) {
                continue;
            }
            const auto audioWriter = audioBuffer[z][dIdx].getWritePointer(0);
            const auto audioReader = currentBuffer[z].data();
            std::memcpy(audioWriter, audioReader + currentPos, (fftSize.load() - currentPos) * sizeof(float));
            if (currentPos > 0) {
                std::memcpy(audioWriter + fftSize.load() - currentPos, audioReader, currentPos * sizeof(float));
            }
        }
        doubleBufferIdx.store((dIdx & BIT_IDX) | BIT_NEWDATA);
        // end writing
    }

    template<typename FloatType>
    void SyncFFTAnalyzer<FloatType>::run() {
        if (!isPrepared.load()) {
            return;
        }
        juce::ScopedNoDenormals noDenormals;
        // read from the double buffer
        auto current = doubleBufferIdx.load();
        if ((current & BIT_NEWDATA) != 0) {
            int newValue;
            do {
                current &= ~BIT_BUSY;
                newValue = (current ^ BIT_IDX) & BIT_IDX;
            } while (!doubleBufferIdx.compare_exchange_weak(current, newValue));
            current = newValue;
        }
        const auto dIdx = static_cast<size_t>((current & BIT_IDX) ^ 1);

        std::vector<size_t> isONVector{};
        if (isON[0].load()) isONVector.push_back(0);
        if (isON[1].load()) isONVector.push_back(1);

        for (const auto &z:isONVector) {
            fftBuffer.copyFrom(0, 0, audioBuffer[z][dIdx],
                               0, 0, audioBuffer[z][dIdx].getNumSamples());

            // calculate fft
            window->multiplyWithWindowingTable(fftBuffer.getWritePointer(0), fftSize.load());
            fft->performFrequencyOnlyForwardTransform(fftBuffer.getWritePointer(0));
            const auto mBuffer = fftBuffer.getReadPointer(0);

            // calculate dB value of each bin and apply decay
            const auto decay = actualDecayRate[z].load();
            for (size_t i = 0; i < fftSize.load() / 2; ++i) {
                const auto currentDB = juce::Decibels::gainToDecibels(
                    2 * mBuffer[i] / static_cast<float>(fftSize.load()), -240.f);
                smoothedDBs[z][i + 1] = currentDB < smoothedDBs[z][i + 1]
                                         ? smoothedDBs[z][i + 1] * decay + currentDB * (1 - decay)
                                         : currentDB;
            }
            smoothedDBs[z][0] = smoothedDBs[z][1] * 2.f;
            smoothedDBs[z][smoothedDBs[z].size() - 1] = smoothedDBs[z][smoothedDBs[z].size() - 2] * 2.f;

            // use makima interpolate dB
            std::vector<float> x = smoothedDBX;
            std::vector<float> y = smoothedDBs[z];

            using boost::math::interpolators::makima;
            const auto spline = makima(std::move(x), std::move(y),
                                       1.f, -1.f);

            preInterplotDBs[z].front() = spline(static_cast<float>(zlIIR::frequencies.front()));
            preInterplotDBs[z].back() = spline(static_cast<float>(zlIIR::frequencies.back()));
            for (size_t i = 0; i < preInterplotDBs[0].size() - 2; ++i) {
                preInterplotDBs[z][i + 1] = spline(static_cast<float>(zlIIR::frequencies[i * preScale]));
            }
        }
        // use cardinal interpolate dB and apply tilt
        const std::array splines{
            boost::math::interpolators::cardinal_quintic_b_spline<float>(
            preInterplotDBs[0].data(), preInterplotDBs[0].size(),
            -static_cast<float>(preScale), static_cast<float>(preScale),
            {0.f, 0.f}, {0.f, 0.f}),
            boost::math::interpolators::cardinal_quintic_b_spline<float>(
            preInterplotDBs[1].data(), preInterplotDBs[1].size(),
            -static_cast<float>(preScale), static_cast<float>(preScale),
            {0.f, 0.f}, {0.f, 0.f})
        };
        const auto tilt = tiltSlope.load() + extraTilt.load();
        for (size_t i = 0; i < interplotDBs[0].size(); ++i) {
            for (const auto &z:isONVector) {
                interplotDBs[z][i].store(splines[z](static_cast<float>(i * 2)) + static_cast<float>(std::log2(
                                          zlIIR::frequencies[i * 2] / 1000)) * tilt);
            }
        }
    }

    template<typename FloatType>
    void SyncFFTAnalyzer<FloatType>::createPath(juce::Path &path1, juce::Path &path2, const juce::Rectangle<float> bound) {
        juce::ScopedNoDenormals noDenormals;
        std::vector<size_t> isONVector{};
        if (isON[0].load()) {
            path1.clear();
            path1.startNewSubPath(bound.getX(), bound.getBottom() + 10.f);
            isONVector.push_back(0);
        }
        if (isON[1].load()) {
            path2.clear();
            path2.startNewSubPath(bound.getX(), bound.getBottom() + 10.f);
            isONVector.push_back(1);
        }
        const std::array<std::reference_wrapper<juce::Path>, 2> paths{path1, path2};

        for (size_t i = 0; i < interplotDBs[0].size(); ++i) {
            const auto x = static_cast<float>(2 * i) / static_cast<float>(zlIIR::frequencies.size() - 1) *
                           bound.getWidth();
            for (const auto &z:isONVector) {
                const auto y = interplotDBs[z][i].load() / minDB * bound.getHeight() + bound.getY();
                paths[z].get().lineTo(x, y);
            }
        }
    }

    template
    class SyncFFTAnalyzer<float>;

    template
    class SyncFFTAnalyzer<double>;
} // zlFFT
