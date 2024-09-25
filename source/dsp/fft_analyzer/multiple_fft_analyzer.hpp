// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLFFT_MULTIPLE_FFT_ANALYZER_HPP
#define ZLFFT_MULTIPLE_FFT_ANALYZER_HPP

#include <boost/math/interpolators/cardinal_quintic_b_spline.hpp>
#include <boost/math/interpolators/makima.hpp>
#include "../../state/state_definitions.hpp"

namespace zlFFT {
    /**
     * a fft analyzer which make sure that multiple FFTs are synchronized in time
     * @tparam FloatType
     */
    template<typename FloatType, size_t FFTNum, size_t PointNum>
    class MultipleFFTAnalyzer final {
    public:
        explicit MultipleFFTAnalyzer() {
            tiltSlope.store(zlState::ffTTilt::slopes[static_cast<size_t>(zlState::ffTTilt::defaultI)]);
            interplotFreqs[0] = minFreq;
            for (size_t i = 1; i < PointNum; ++i) {
                const float temp = static_cast<float>(i) / static_cast<float>(PointNum - 1) * (
                                       maxFreqLog2 - minFreqLog2) + minFreqLog2;
                interplotFreqs[i] = std::pow(2.f, temp);
            }
            reset();
        }

        ~MultipleFFTAnalyzer() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            juce::GenericScopedLock lock(spinLock);
            sampleRate.store(static_cast<float>(spec.sampleRate));
            int extraOrder = 0;
            if (spec.sampleRate >= 50000) {
                extraOrder = 1;
            } else if (spec.sampleRate >= 100000) {
                extraOrder = 2;
            }
            setOrder(extraOrder + defaultFFTOrder);
            reset();
            isPrepared.store(true);
        }

        void reset() {
            for (size_t i = 0; i < FFTNum; ++i) {
                for (size_t j = 0; j < PointNum; ++j) {
                    interplotDBs[i][j].store(minDB * 2.f);
                }
            }
            toClear.store(true);
            toClearFFT.store(true);
        }

        void setOrder(int fftOrder) {
            fft = std::make_unique<juce::dsp::FFT>(fftOrder);
            window = std::make_unique<
                juce::dsp::WindowingFunction<float> >(static_cast<size_t>(fft->getSize()),
                                                      juce::dsp::WindowingFunction<float>::hann,
                                                      true);
            fftSize.store(static_cast<size_t>(fft->getSize()));

            deltaT.store(sampleRate.load() / static_cast<float>(fftSize.load()));
            decayRate.store(zlState::ffTSpeed::speeds[static_cast<size_t>(zlState::ffTSpeed::defaultI)]);

            smoothedFreqs[0] = deltaT.load() * (-.5f);
            for (size_t idx = 1; idx < smoothedFreqs.size(); ++idx) {
                smoothedFreqs[idx] = smoothedFreqs[idx - 1] + deltaT.load();
            }

            const auto tempSize = fft->getSize();
            fftBuffer.resize(static_cast<size_t>(tempSize * 2));
            abstractFIFO.setTotalSize(tempSize);
            for (size_t i = 0; i < FFTNum; ++i) {
                sampleFIFOs[i].resize(static_cast<size_t>(tempSize));
                circularBuffers[i].resize(static_cast<size_t>(tempSize));
            }
        }

        /**
         * put input samples into FIFOs
         * @param buffers
         */
        void process(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, FFTNum> buffers) {
            const int freeSpace = std::min(abstractFIFO.getFreeSpace(), buffers[0].get().getNumSamples());
            if (freeSpace == 0) { return; }
            const auto scope = abstractFIFO.write(freeSpace);
            for (size_t i = 0; i < FFTNum; ++i) {
                if (!isON[i].load()) continue;
                int j = 0;
                const auto &buffer{buffers[i]};
                const FloatType avgScale = FloatType(1) / static_cast<FloatType>(buffer.getNumChannels());
                int shift = 0;
                for (; j < scope.blockSize1; ++j) {
                    FloatType sample{0};
                    for (int channel = 0; channel < buffer.get().getNumChannels(); ++channel) {
                        sample += std::abs(buffer.get().getSample(channel, j));
                    }
                    sample *= avgScale;
                    sampleFIFOs[i][static_cast<size_t>(shift + scope.startIndex1)] = static_cast<float>(sample);
                    shift += 1;
                }
                shift = 0;
                for (; j < scope.blockSize1 + scope.blockSize2; ++j) {
                    FloatType sample{0};
                    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                        sample += buffer.getSample(channel, j);
                    }
                    sample *= avgScale;
                    sampleFIFOs[i][static_cast<size_t>(shift + scope.startIndex2)] = static_cast<float>(sample);
                    shift += 1;
                }
            }
        }

        /**
         * run the forward FFT
         */
        void run() {
            if (!isPrepared.load()) {
                return;
            }
            std::vector<size_t> isONVector{};
            for (size_t i = 0; i < FFTNum; ++i) {
                if (isON[i].load()) isONVector.push_back(i);
            }

            juce::ScopedNoDenormals noDenormals; {
                const int numReady = abstractFIFO.getNumReady();
                const auto scope = abstractFIFO.read(numReady);
                const size_t numReplace = circularBuffers[0].size() - static_cast<size_t>(numReady);
                for (const auto &i: isONVector) {
                    const auto &circularBuffer{circularBuffers[i]};
                    const auto &sampleFIFO{sampleFIFOs[i]};
                    size_t j = 0;
                    for (; j < numReplace; ++j) {
                        circularBuffer[j] = circularBuffer[j + static_cast<size_t>(numReady)];
                    }
                    int shift = 0;
                    for (; j < numReplace + static_cast<size_t>(scope.blockSize1); ++j) {
                        circularBuffer[j] = sampleFIFO[static_cast<size_t>(shift + scope.startIndex1)];
                        shift += 1;
                    }
                    shift = 0;
                    for (; j < numReplace + static_cast<size_t>(scope.blockSize1 + scope.blockSize2); ++j) {
                        circularBuffer[j] = sampleFIFO[static_cast<size_t>(shift + scope.startIndex2)];
                        shift += 1;
                    }
                }
            } {
                for (const auto &i: isONVector) {
                    std::copy(circularBuffers[i].begin(), circularBuffers[i].end(), fftBuffer.begin());
                    window->multiplyWithWindowingTable(fftBuffer.data(), fftSize.load());
                    fft->performFrequencyOnlyForwardTransform(fftBuffer.data());
                    const auto decay = actualDecayRate[i].load();
                    const auto &smoothedDB{smoothedDBs[i]};
                    for (size_t j = 0; j < smoothedFreqs.size(); ++j) {
                        const auto currentDB = juce::Decibels::gainToDecibels(
                            fftBuffer[j] / static_cast<float>(fftBuffer.size()), -240.f);
                        smoothedDB[j + 1] = currentDB < smoothedDB[j + 1]
                                                ? smoothedDB[j + 1] * decay + currentDB * (1 - decay)
                                                : currentDB;
                    }
                    smoothedDB[0] = smoothedDB[1] * 2.f;
                    smoothedDB[smoothedDB.size() - 1] = smoothedDB[smoothedDB.size() - 2] * 2.f;

                    std::vector<float> x{smoothedFreqs.begin(), smoothedFreqs.end()};
                    std::vector<float> y = smoothedDB;
                    using boost::math::interpolators::makima;
                    const auto spline = makima(std::move(x), std::move(y),
                                               1.f, -1.f);

                    preInterplotDBs[i].front() = spline(minFreq);
                    preInterplotDBs[i].back() = spline(maxFreq);
                    for (size_t j = 1; j < preInterplotDBs[i].size() - 1; ++j) {
                        preInterplotDBs[i][j] = spline(interplotFreqs[j * 2]);
                    }
                }
            } {
                std::vector<std::unique_ptr<boost::math::interpolators::cardinal_quintic_b_spline<float> > > splines;
                for (const auto &i: isONVector) {
                    splines.push_back(std::make_unique<boost::math::interpolators::cardinal_quintic_b_spline<float> >(
                        preInterplotDBs[i].data(), preInterplotDBs[i].size(),
                        0.f, 2.f, {0.f, 0.f}, {0.f, 0.f}));
                }
                const float totalTilt = tiltSlope.load() + extraTilt.load();
                const float tiltShiftTotal = (maxFreqLog2 - minFreqLog2) * totalTilt;
                const float tiltShiftDelta = tiltShiftTotal / static_cast<float>(PointNum - 1);
                float tiltShift = tiltShiftTotal * .5f;
                for (size_t idx = 0; idx < PointNum; ++idx) {
                    size_t splineIdx = 0;
                    for (const auto &i: isONVector) {
                        const auto &spline{splines[splineIdx]};
                        splineIdx += 1;
                        interplotDBs[i][idx].store(tiltShift + spline->operator()(static_cast<float>(idx)));
                    }
                    tiltShift += tiltShiftDelta;
                }
            }
        }

        void createPath(std::array<std::reference_wrapper<juce::Path>, FFTNum> paths,
                        const juce::Rectangle<float> bound) {
            for (auto &p: paths) {
                p.get().clear();
            }
            std::vector<size_t> isONVector{};
            for (size_t i = 0; i < FFTNum; ++i) {
                if (isON[i].load()) isONVector.push_back(i);
            }
            const float width = bound.getWidth(), height = bound.getHeight(), boundY = bound.getY();
            for (const auto &i: isONVector) {
                const auto &path{paths[i]};
                path.get().startNewSubPath(bound.getX(), bound.getBottom() + 10.f);
                for (size_t idx = 0; idx < PointNum; ++idx) {
                    const auto x = static_cast<float>(idx) / static_cast<float>(PointNum - 1) * width;
                    const auto y = interplotDBs[i][idx].load() / minDB * height + boundY;
                    path.get().lineTo(x, y);
                }
            }
        }

    private:
        juce::SpinLock spinLock;
        static constexpr size_t defaultFFTOrder = 11;
        static constexpr float minFreq = 10.f, maxFreq = 22000.f, minDB = -72.f;
        static constexpr float minFreqLog2 = 3.321928094887362f;
        static constexpr float maxFreqLog2 = 14.425215903299383f;

        std::array<std::vector<float>, FFTNum> sampleFIFOs;
        std::array<std::vector<float>, FFTNum> circularBuffers;
        juce::AbstractFifo abstractFIFO{1};

        std::vector<float> fftBuffer;
        std::array<float, (1 << defaultFFTOrder) + 2> smoothedFreqs{};
        std::array<std::array<float, (1 << defaultFFTOrder) + 2>, FFTNum> smoothedDBs{};
        std::array<float, PointNum> interplotFreqs{};
        std::array<std::array<float, PointNum / 2 + 1>, FFTNum> preInterplotDBs{};
        std::array<std::array<std::atomic<float>, PointNum>, FFTNum> interplotDBs{};
        std::atomic<float> deltaT, decayRate, refreshRate{60}, tiltSlope;
        std::array<std::atomic<float>, FFTNum> decayRates{}, actualDecayRate{};
        std::atomic<float> extraTilt{0.f}, extraSpeed{1.f};

        std::unique_ptr<juce::dsp::FFT> fft;
        std::unique_ptr<juce::dsp::WindowingFunction<float> > window;
        std::atomic<size_t> fftSize;

        std::atomic<float> sampleRate;
        std::atomic<bool> toClear{false}, toClearFFT{true};
        std::atomic<bool> isPrepared{false};

        std::array<std::atomic<bool>, FFTNum> isON{};

        inline float indexToX(const size_t index, const juce::Rectangle<float> bounds) const {
            const auto portion = (static_cast<float>(index) + .5f) / static_cast<float>(fftSize.load());
            return bounds.getX() +
                   bounds.getWidth() * std::log(sampleRate * portion / minFreq) / std::log(maxFreq / minFreq);
        }

        inline float binToY(const float bin, const juce::Rectangle<float> bounds) const {
            const auto db = juce::Decibels::gainToDecibels(bin, -240.f);
            return bounds.getY() + (db / minDB) * bounds.getHeight();
        }
    };
}

#endif //ZLFFT_MULTIPLE_FFT_ANALYZER_HPP
