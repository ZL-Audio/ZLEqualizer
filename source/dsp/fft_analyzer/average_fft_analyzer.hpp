// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../state/state_definitions.hpp"
#include "../interpolation/interpolation.hpp"

namespace zlFFT {
    /**
         * a fft analyzer which make sure that multiple FFTs are synchronized in time
         * @tparam FloatType the float type of input audio buffers
         * @tparam FFTNum the number of FFTs
         * @tparam PointNum the number of output points
         */
    template<typename FloatType, size_t FFTNum, size_t PointNum>
    class AverageFFTAnalyzer final {
    private:
        juce::SpinLock spinLock;

    public:
        static constexpr float minFreq = 10.f, maxFreq = 22000.f, minDB = -72.f;
        static constexpr float minFreqLog2 = 3.321928094887362f;
        static constexpr float maxFreqLog2 = 14.425215903299383f;
        static constexpr float tiltSlope = zlState::ffTTilt::slopes[static_cast<size_t>(zlState::ffTTilt::defaultI)];
        static constexpr float tiltShiftTotal = (maxFreqLog2 - minFreqLog2) * tiltSlope;

    public:
        explicit AverageFFTAnalyzer(const size_t fftOrder = 12) {
            defaultFFTOrder = fftOrder;
            binSize = (1 << (defaultFFTOrder - 1)) + 1;

            for (auto &db: smoothedDBs) {
                db.resize(binSize);
            }

            prepareAkima();

            interplotFreqs[0] = minFreq;
            for (size_t i = 1; i < PointNum; ++i) {
                const float temp = static_cast<float>(i) / static_cast<float>(PointNum - 1) * (
                                       maxFreqLog2 - minFreqLog2) + minFreqLog2;
                interplotFreqs[i] = std::pow(2.f, temp);
            }
            for (auto &db: interplotDBs) {
                std::fill(db.begin(), db.end(), minDB * 2.f);
            }
            reset();
        }

        ~AverageFFTAnalyzer() = default;

        void prepareAkima() {
            std::vector<size_t> seqInputIndices{};
            seqInputIndices.push_back(0);
            size_t i = 1, i0 = 1;
            const float delta = std::pow(
                static_cast<float>(binSize), .75f / static_cast<float>(PointNum));
            while (i < binSize - 1) {
                while (static_cast<float>(i) / static_cast<float>(i0) < delta) {
                    i += 1;
                    if (i >= binSize - 1) {
                        break;
                    }
                }
                i0 = i;
                seqInputIndices.push_back(i);
            }

            seqInputStarts.reserve(seqInputIndices.size());
            seqInputEnds.reserve(seqInputIndices.size());
            seqInputStarts.push_back(0);
            seqInputEnds.push_back(1);
            for (size_t idx = 1; idx < seqInputIndices.size() - 1; ++idx) {
                seqInputStarts.push_back(seqInputEnds.back());
                seqInputEnds.push_back(
                    static_cast<std::vector<float>::difference_type>(
                        seqInputIndices[idx] + seqInputIndices[idx + 1]) / 2);
            }
            seqInputStarts.push_back(seqInputEnds.back());
            seqInputEnds.push_back(static_cast<std::vector<float>::difference_type>(binSize) - 1);

            seqInputFreqs.resize(seqInputIndices.size());
            seqInputDBs.resize(seqInputIndices.size());
            seqAkima = std::make_unique<zlInterpolation::SeqMakima<float> >(
                seqInputFreqs.data(), seqInputDBs.data(), seqInputFreqs.size(), 0.f, 0.f);
        }

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
            toReset.store(true);
            toResetOutput.store(true);
            for (auto &f: readyFlags) {
                f.store(false);
            }
        }

        void setOrder(int fftOrder) {
            fft = std::make_unique<juce::dsp::FFT>(fftOrder);
            window = std::make_unique<
                juce::dsp::WindowingFunction<float> >(static_cast<size_t>(fft->getSize()),
                                                      juce::dsp::WindowingFunction<float>::hann,
                                                      true);
            fftSize.store(static_cast<size_t>(fft->getSize()));

            const float deltaT = sampleRate.load() / static_cast<float>(fftSize.load());

            const auto currentDeltaT = .5f * deltaT;
            for (size_t idx = 0; idx < seqInputFreqs.size(); ++idx) {
                seqInputFreqs[idx] = static_cast<float>(seqInputStarts[idx] + seqInputEnds[idx] - 1) * currentDeltaT;
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
            readyNum.store(tempSize / 2);
        }

        /**
         * put input samples into FIFOs
         * @param buffers
         */
        void process(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, FFTNum> buffers) {
            int freeSpace = abstractFIFO.getFreeSpace();
            for (size_t i = 0; i < FFTNum; ++i) {
                if (!isON[i].load()) { continue; }
                freeSpace = std::min(freeSpace, buffers[i].get().getNumSamples());
            }
            if (freeSpace == 0) { return; }
            const auto scope = abstractFIFO.write(freeSpace);
            for (size_t i = 0; i < FFTNum; ++i) {
                if (!isON[i].load()) { continue; }
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
            if (!getReadyForNextFFT()) {
                return;
            }
            std::vector<size_t> isONVector{};
            for (size_t i = 0; i < FFTNum; ++i) {
                if (isON[i].load()) isONVector.push_back(i);
            }
            // collect data from FIFO
            juce::ScopedNoDenormals noDenormals; {
                const int numReady = abstractFIFO.getNumReady();
                const auto scope = abstractFIFO.read(numReady);
                const size_t numReplace = circularBuffers[0].size() - static_cast<size_t>(numReady);
                for (const auto &i: isONVector) {
                    if (!isON[i].load()) { continue; }
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
                // reset if required
                if (toReset.exchange(false)) {
                    for (size_t i = 0; i < FFTNum; ++i) {
                        std::fill(smoothedDBs[i].begin(), smoothedDBs[i].end(), -36.f);
                        currentNum[i] = 0.01f;
                    }
                }
                // calculate FFT and average results
                for (const auto &i: isONVector) {
                    // calculate RMS of current buffer
                    const float ms = std::inner_product(circularBuffers[i].begin(), circularBuffers[i].end(),
                                                        circularBuffers[i].begin(), 0.f)
                                     / static_cast<float>(circularBuffers[i].size());
                    const float rms = juce::Decibels::gainToDecibels(ms, -160.f) * 0.5f;
                    if (rms < -80.f) { continue; }
                    // calculate loudness weighting
                    const auto weight = calculateWeight(rms);
                    currentNum[i] += weight;
                    const auto newWeight = weight / currentNum[i];
                    const auto oldWeight = 1.f - newWeight;
                    // perform FFT
                    std::copy(circularBuffers[i].begin(), circularBuffers[i].end(), fftBuffer.begin());
                    window->multiplyWithWindowingTable(fftBuffer.data(), fftSize.load());
                    fft->performFrequencyOnlyForwardTransform(fftBuffer.data());
                    auto &smoothedDB{smoothedDBs[i]};
                    const auto ampScale = 2.f / static_cast<float>(fftBuffer.size());
                    // calculate rms weighted average dBs
                    for (size_t j = 0; j < smoothedDB.size(); ++j) {
                        const auto currentDB = juce::Decibels::gainToDecibels(ampScale * fftBuffer[j], -120.f);
                        smoothedDB[j] = smoothedDB[j] * oldWeight + currentDB * newWeight;
                    }
                    // calculate seq-akima input dBs
                    for (size_t j = 0; j < seqInputDBs.size(); ++j) {
                        const auto startIdx = seqInputStarts[j];
                        const auto endIdx = seqInputEnds[j];
                        seqInputDBs[j] = std::reduce(
                                             smoothedDB.begin() + startIdx,
                                             smoothedDB.begin() + endIdx) / static_cast<float>(endIdx - startIdx);
                    }
                    // interpolate via seq-akima
                    seqAkima->prepare();
                    seqAkima->eval(interplotFreqs.data(), preInterplotDBs[i].data(), PointNum);
                }
            } {
                const float tiltShiftDelta = tiltShiftTotal / static_cast<float>(PointNum - 1);
                // apply tilt
                for (const auto &i: isONVector) {
                    if (readyFlags[i].load() == false) {
                        float tiltShift = -tiltShiftTotal * .5f;
                        for (size_t idx = 0; idx < PointNum; ++idx) {
                            interplotDBs[i][idx] = tiltShift + preInterplotDBs[i][idx];
                            tiltShift += tiltShiftDelta;
                        }
                    }
                    readyFlags[i].store(true);
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

            for (const auto &i: isONVector) {
                if (readyFlags[i].load() == true) {
                    readyDBs[i] = interplotDBs[i];
                }
                readyFlags[i].store(false);
            }
            const float width = bound.getWidth(), height = bound.getHeight(), boundY = bound.getY();
            for (const auto &i: isONVector) {
                const auto &path{paths[i]};
                path.get().startNewSubPath(bound.getX(), bound.getBottom() + 10.f);
                for (size_t idx = 0; idx < PointNum; ++idx) {
                    const auto x = static_cast<float>(idx) / static_cast<float>(PointNum - 1) * width;
                    const auto y = replaceWithFinite(readyDBs[i][idx] / minDB * height + boundY);
                    path.get().lineTo(x, y);
                }
            }
        }

        void setON(const size_t idx, const bool f) {
            isON[idx].store(f);
        }

        inline size_t getFFTSize() const { return fftSize.load(); }

        std::array<float, PointNum> &getInterplotDBs(const size_t i) {
            if (readyFlags[i].load() == true) {
                readyDBs[i] = interplotDBs[i];
                readyFlags[i].store(false);
                toResetOutput.store(false);
            } else if (toResetOutput.load()) {
                std::fill(readyDBs[i].begin(), readyDBs[i].end(), 2.f * minDB);
            }
            return readyDBs[i];
        }

        bool getReadyForNextFFT() const {
            return abstractFIFO.getNumReady() >= readyNum.load();
        }

        void setWeight(const float x) {
            loudnessWeightAlpha.store(std::clamp(x, 0.f, 1.f));
        }

    private:
        size_t defaultFFTOrder = 12;
        size_t binSize = (1 << (defaultFFTOrder - 1)) + 1;

        std::array<std::vector<float>, FFTNum> sampleFIFOs;
        std::array<std::vector<float>, FFTNum> circularBuffers;
        juce::AbstractFifo abstractFIFO{1};

        std::vector<float> fftBuffer;

        // smooth dbs over time
        std::array<std::vector<float>, FFTNum> smoothedDBs{};
        std::array<float, FFTNum> currentNum{};
        std::atomic<float> loudnessWeightAlpha{0.5f};
        // smooth dbs over high frequency for Akimas input
        std::vector<float> seqInputFreqs{};
        std::vector<std::vector<float>::difference_type> seqInputStarts, seqInputEnds;
        std::vector<size_t> setInputIndices;
        std::vector<float> seqInputDBs{};

        std::unique_ptr<zlInterpolation::SeqMakima<float> > seqAkima;

        std::array<float, PointNum> interplotFreqs{};
        std::array<std::array<float, PointNum>, FFTNum> preInterplotDBs{};
        std::array<std::array<float, PointNum>, FFTNum> interplotDBs{};
        std::array<std::array<float, PointNum>, FFTNum> readyDBs{};
        std::array<std::atomic<bool>, FFTNum> readyFlags;
        std::atomic<int> readyNum{std::numeric_limits<int>::max()};

        std::unique_ptr<juce::dsp::FFT> fft;
        std::unique_ptr<juce::dsp::WindowingFunction<float> > window;
        std::atomic<size_t> fftSize;

        std::atomic<float> sampleRate;
        std::atomic<bool> toReset{true}, toResetOutput{true};
        std::atomic<bool> isPrepared{false};

        std::array<std::atomic<bool>, FFTNum> isON;

        static inline float replaceWithFinite(const float x) {
            return std::isfinite(x) ? x : 100000.f;
        }

        float calculateWeight(const float rms) const {
            const auto t = loudnessWeightAlpha.load();
            return (1.f / 80.f * rms + 1.f) * (1.f - t) + std::exp(0.1f * rms) * t;
        }
    };
}
