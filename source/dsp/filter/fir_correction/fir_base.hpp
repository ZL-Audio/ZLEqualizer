// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_dsp/juce_dsp.h>

#include "../../fft/fft.hpp"
#include "../../vector/vector.hpp"

namespace zlFilter {
    template<typename FloatType, size_t defaultFFTOrder = 10>
    class FIRBase {
    public:
        virtual ~FIRBase() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            if (spec.sampleRate <= 50000) {
                setOrder(static_cast<size_t>(spec.numChannels), defaultFFTOrder);
            } else if (spec.sampleRate <= 100000) {
                setOrder(static_cast<size_t>(spec.numChannels), defaultFFTOrder + 1);
            } else if (spec.sampleRate <= 200000) {
                setOrder(static_cast<size_t>(spec.numChannels), defaultFFTOrder + 2);
            } else {
                setOrder(static_cast<size_t>(spec.numChannels), defaultFFTOrder + 3);
            }
        }

        void reset() {
            pos = 0;
            count = 0;
            for (auto &fifo: inputFIFOs) {
                fifo.resize(fftSize);
                std::fill(fifo.begin(), fifo.end(), 0.f);
            }
            for (auto &fifo: outputFIFOs) {
                fifo.resize(fftSize);
                std::fill(fifo.begin(), fifo.end(), 0.f);
            }
            std::fill(fftData.begin(), fftData.end(), 0.f);
        }

        template<bool isBypassed = false>
        void process(juce::AudioBuffer<FloatType> &buffer) {
            for (size_t i = 0; i < static_cast<size_t>(buffer.getNumSamples()); ++i) {
                for (size_t channel = 0; channel < static_cast<size_t>(buffer.getNumChannels()); ++channel) {
                    auto writePointer = buffer.getWritePointer(static_cast<int>(channel), static_cast<int>(i));
                    inputFIFOs[channel][pos] = static_cast<float>(*writePointer);
                    *writePointer = static_cast<FloatType>(outputFIFOs[channel][pos]);
                    outputFIFOs[channel][pos] = FloatType(0);
                }

                pos += 1;
                if (pos == fftSize) {
                    pos = 0;
                }
                count += 1;
                if (count == hopSize) {
                    count = 0;
                    processFrame<isBypassed>();
                }
            }
        }

        int getLatency() const { return latency.load(); }

    protected:
        zlFFT::KFREngine<float> fft;
        zlFFT::WindowFunction<float> window;

        size_t fftOrder = defaultFFTOrder;
        size_t fftSize = static_cast<size_t>(1) << fftOrder;
        size_t numBins = fftSize / 2 + 1;
        size_t overlap = 4; // 75% overlap
        size_t hopSize = fftSize / overlap;
        static constexpr float windowCorrection = 2.0f / 3.0f;
        static constexpr float bypassCorrection = 1.0f / 4.0f;
        // counts up until the next hop.
        size_t count = 0;
        // write position in input FIFO and read position in output FIFO.
        size_t pos = 0;
        // circular buffers for incoming and outgoing audio data.
        std::vector<kfr::univector<float> > inputFIFOs, outputFIFOs;
        // circular FFT working space which contains interleaved complex numbers.
        kfr::univector<float> fftData;

        size_t fftDataPos = 0;
        std::atomic<int> latency{0};


        void setFFTOrder(const size_t channelNum, const size_t order) {
            fftOrder = order;
            fftSize = static_cast<size_t>(1) << fftOrder;
            numBins = fftSize / 2 + 1;
            hopSize = fftSize / overlap;
            latency.store(static_cast<int>(fftSize));

            fft.setOrder(fftOrder);
            window.setWindow(fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false);

            inputFIFOs.resize(channelNum);
            outputFIFOs.resize(channelNum);
            fftData.resize(fftSize * 2);
        }

        template<bool isBypassed = false>
        void processFrame() {
            for (size_t idx = 0; idx < inputFIFOs.size(); ++idx) {
                const auto *inputPtr = inputFIFOs[idx].data();
                auto *fftPtr = fftData.data();

                // Copy the input FIFO into the FFT working space in two parts.
                std::memcpy(fftPtr, inputPtr + pos, (fftSize - pos) * sizeof(float));
                if (pos > 0) {
                    std::memcpy(fftPtr + fftSize - pos, inputPtr, pos * sizeof(float));
                }

                if (!isBypassed) {
                    window.multiply(fftPtr, fftSize);

                    fft.forward(fftPtr, fftPtr);
                    processSpectrum();
                    fft.backward(fftPtr, fftPtr);

                    window.multiply(fftPtr, fftSize);
                    for (size_t i = 0; i < fftSize; ++i) {
                        fftPtr[i] *= windowCorrection;
                    }
                } else {
                    for (size_t i = 0; i < fftSize; ++i) {
                        fftPtr[i] *= bypassCorrection;
                    }
                }

                for (size_t i = 0; i < pos; ++i) {
                    outputFIFOs[idx][i] += fftData[i + fftSize - pos];
                }
                for (size_t i = 0; i < fftSize - pos; ++i) {
                    outputFIFOs[idx][i + pos] += fftData[i];
                }
            }
        }

        virtual void setOrder(size_t channelNum, size_t order) = 0;

        virtual void processSpectrum() = 0;
    };
}
