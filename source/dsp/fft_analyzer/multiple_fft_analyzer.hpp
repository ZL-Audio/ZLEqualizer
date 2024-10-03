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

#include "../../state/state_definitions.hpp"
#include <boost/math/interpolators/cardinal_quintic_b_spline.hpp>
#include <boost/math/interpolators/makima.hpp>

namespace zlFFT {
    /**
     * a fft analyzer which make sure that multiple FFTs are synchronized in time
     * @tparam FloatType
     */
    template<typename FloatType, size_t FFTNum, size_t PointNum>
    class MultipleFFTAnalyzer final {
    private:
        juce::SpinLock spinLock;
        static constexpr size_t defaultFFTOrder = 12;
        static constexpr float minFreq = 10.f, maxFreq = 22000.f, minDB = -72.f;
        static constexpr float minFreqLog2 = 3.321928094887362f;
        static constexpr float maxFreqLog2 = 14.425215903299383f;

    public:
        explicit MultipleFFTAnalyzer() {
            static_assert(PointNum % 2 == 1);
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
            if (spec.sampleRate <= 50000) {
                setOrder(static_cast<int>(defaultFFTOrder));
            } else if (spec.sampleRate <= 100000) {
                setOrder(static_cast<int>(defaultFFTOrder) + 1);
            } else if (spec.sampleRate <= 200000) {
                setOrder(static_cast<int>(defaultFFTOrder) + 2);
            } else {
                setOrder(static_cast<int>(defaultFFTOrder) + 3);
            }
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

            const auto currentDeltaT = deltaT.load();
            smoothedFreqs[0] = 0.f;
            for (size_t idx = 1; idx < smoothedFreqs.size(); ++idx) {
                smoothedFreqs[idx] = smoothedFreqs[idx - 1] + currentDeltaT;
            }
            for (size_t i = 0; i < FFTNum; ++i) {
                std::fill(smoothedDBs[i].begin(), smoothedDBs[i].end(), minDB * 2.f);
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
            int freeSpace = abstractFIFO.getFreeSpace();
            for (size_t i = 0; i < FFTNum; ++i) {
                if (!isON[i].load()) continue;
                freeSpace = std::min(freeSpace, buffers[i].get().getNumSamples());
            }
            if (freeSpace == 0) { return; }
            const auto scope = abstractFIFO.write(freeSpace);
            for (size_t i = 0; i < FFTNum; ++i) {
                if (!isON[i].load()) continue;
                int j = 0;
                const auto &buffer{buffers[i]};
                const FloatType avgScale = FloatType(1) / static_cast<FloatType>(buffer.get().getNumChannels());
                int shift = 0;
                for (; j < scope.blockSize1; ++j) {
                    FloatType sample{0};
                    for (int channel = 0; channel < buffer.get().getNumChannels(); ++channel) {
                        sample += buffer.get().getSample(channel, j);
                    }
                    sample *= avgScale;
                    sampleFIFOs[i][static_cast<size_t>(shift + scope.startIndex1)] = static_cast<float>(sample);
                    shift += 1;
                }
                shift = 0;
                for (; j < scope.blockSize1 + scope.blockSize2; ++j) {
                    FloatType sample{0};
                    for (int channel = 0; channel < buffer.get().getNumChannels(); ++channel) {
                        sample += buffer.get().getSample(channel, j);
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
            juce::GenericScopedLock lock(spinLock);
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
                    auto &circularBuffer{circularBuffers[i]};
                    auto &sampleFIFO{sampleFIFOs[i]};
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
                    auto &smoothedDB{smoothedDBs[i]};
                    const auto ampScale = 2.f / static_cast<float>(fftBuffer.size());
                    for (size_t j = 0; j < smoothedFreqs.size(); ++j) {
                        const auto currentDB = juce::Decibels::gainToDecibels(ampScale * fftBuffer[j], -240.f);
                        smoothedDB[j] = currentDB < smoothedDB[j]
                                            ? smoothedDB[j] * decay + currentDB * (1 - decay)
                                            : currentDB;
                    }

                    std::vector<float> x{smoothedFreqs.begin(), smoothedFreqs.end()};
                    std::vector<float> y{smoothedDB.begin(), smoothedDB.end()};
                    using boost::math::interpolators::makima;
                    const auto spline = makima(std::move(x), std::move(y), 0.f, 0.f);

                    for (size_t j = 0; j < preInterplotDBs[i].size(); ++j) {
                        preInterplotDBs[i][j] = spline(interplotFreqs[j * 2]);
                    }
                }
            } {
                const float totalTilt = tiltSlope.load() + extraTilt.load();
                const float tiltShiftTotal = (maxFreqLog2 - minFreqLog2) * totalTilt;
                const float tiltShiftDelta = tiltShiftTotal / static_cast<float>(PointNum - 1);
                for (const auto &i: isONVector) {
                    auto spline = boost::math::interpolators::cardinal_quintic_b_spline<float>(
                        preInterplotDBs[i].data(), preInterplotDBs[i].size(),
                        0.f, 2.f, {0.f, 0.f}, {0.f, 0.f});
                    float tiltShift = -tiltShiftTotal * .5f;
                    for (size_t idx = 0; idx < PointNum; ++idx) {
                        interplotDBs[i][idx].store(tiltShift + spline(static_cast<float>(idx)));
                        tiltShift += tiltShiftDelta;
                    }
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

        void setON(std::array<bool, FFTNum> fs) {
            for (size_t i = 0; i < FFTNum; ++i) {
                isON[i].store(fs[i]);
            }
        }

        inline size_t getFFTSize() const { return fftSize.load(); }

        void setDecayRate(const size_t idx, const float x) {
            decayRates[idx].store(x);
            updateActualDecayRate();
        }

        void setRefreshRate(const float x) {
            refreshRate.store(x);
            updateActualDecayRate();
        }

        void setTiltSlope(const float x) { tiltSlope.store(x); }

        void setExtraTilt(const float x) { extraTilt.store(x); }

        void setExtraSpeed(const float x) {
            extraSpeed.store(x);
            updateActualDecayRate();
        }

        void updateActualDecayRate() {
            for (size_t i = 0; i < FFTNum; ++i) {
                const auto x = 1 - (1 - decayRates[i].load()) * extraSpeed.load();
                actualDecayRate[i].store(std::pow(x, 23.4375f / refreshRate.load()));
            }
        }

        std::array<std::atomic<float>, PointNum> &getInterplotDBs(const size_t i) { return interplotDBs[i]; }

    private:
        std::array<std::vector<float>, FFTNum> sampleFIFOs;
        std::array<std::vector<float>, FFTNum> circularBuffers;
        juce::AbstractFifo abstractFIFO{1};

        std::vector<float> fftBuffer;
        std::array<float, (1 << defaultFFTOrder) + 1> smoothedFreqs{};
        std::array<std::array<float, (1 << defaultFFTOrder) + 1>, FFTNum> smoothedDBs{};
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
