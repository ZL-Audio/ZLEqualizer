// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLFILTER_PROTOTYPE_CORRECTION_HPP
#define ZLFILTER_PROTOTYPE_CORRECTION_HPP

#include "../iir_filter/iir_filter.hpp"
#include "../ideal_filter/ideal_filter.hpp"
#include "../../container/array.hpp"

namespace zlFilter {
    template<typename FloatType, size_t FilterNum, size_t FilterSize>
    class PrototypeCorrection {
    public:
        static constexpr size_t defaultFFTOrder = 9;

        PrototypeCorrection(std::array<IIR<FloatType, FilterSize>, FilterNum> &iir,
                            std::array<Ideal<FloatType, FilterSize>, FilterNum> &ideal,
                            zlContainer::FixedMaxSizeArray<size_t, FilterNum> &indices)
            : iirFs(iir), idealFs(ideal), filterIndices(indices) {
        }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            if (spec.sampleRate <= 50000) {
                setOrder(defaultFFTOrder);
            } else if (spec.sampleRate <= 100000) {
                setOrder(defaultFFTOrder + 1);
            } else {
                setOrder(defaultFFTOrder + 2);
            }
            const auto delta = pi / static_cast<double>(corrections.size() - 1);
            double w = 0.f;
            for (size_t i = 0; i < corrections.size(); ++i) {
                wis1[i] = std::complex<float>(0.f, static_cast<float>(w));
                wis2[i] = std::exp(std::complex<float>(0.f, -static_cast<float>(w)));
                w += delta;
            }
        }

        void process(juce::AudioBuffer<FloatType> &buffer) {
            auto writePointer = buffer.getWritePointer(0);
            for (size_t i = 0; i < static_cast<size_t>(buffer.getNumSamples()); ++i) {
                inputFifo[pos] = static_cast<float>(writePointer[i]);
                writePointer[i] = static_cast<FloatType>(outputFifo[pos]);
                outputFifo[pos] = FloatType(0);
                pos += 1;
                if (pos == fftSize) {
                    pos = 0;
                }
                count += 1;
                if (count == hopSize) {
                    count = 0;
                    processFrame();
                }
            }
        }

        int getLatency() const { return latency.load(); }

        void setToUpdate() { toUpdate.store(true); }

    private:
        std::array<IIRIdle<FloatType, FilterSize>, FilterNum> &iirFs;
        std::array<Ideal<FloatType, FilterSize>, FilterNum> &idealFs;
        zlContainer::FixedMaxSizeArray<size_t, FilterNum> &filterIndices;
        std::atomic<bool> toUpdate{true};

        std::vector<std::complex<FloatType> > iirTotalResponse, idealTotalResponse;
        // prototype corrections
        std::vector<std::complex<FloatType> > corrections;
        std::vector<std::complex<FloatType> > wis1, wis2;

        std::unique_ptr<juce::dsp::FFT> fft;
        std::unique_ptr<juce::dsp::WindowingFunction<float> > window;

        size_t fftOrder = defaultFFTOrder;
        size_t fftSize = static_cast<size_t>(1) << fftOrder;
        size_t numBins = fftSize / 2 + 1;
        size_t overlap = 4; // 75% overlap
        size_t hopSize = fftSize / overlap;
        static constexpr float windowCorrection = 2.0f / 3.0f;
        // counts up until the next hop.
        size_t count = 0;
        // write position in input FIFO and read position in output FIFO.
        size_t pos = 0;
        // circular buffers for incoming and outgoing audio data.
        std::vector<float> inputFifo, outputFifo;
        // circular FFT working space which contains interleaved complex numbers.
        std::vector<float> fftData;
        size_t fftDataPos = 0;

        std::atomic<int> latency{0};

        void setOrder(const size_t order) {
            fftOrder = order;
            fftSize = static_cast<size_t>(1) << fftOrder;
            numBins = fftSize / 2 + 1;
            hopSize = fftSize / overlap;
            latency.store(static_cast<int>(fftSize));

            fft = std::make_unique<juce::dsp::FFT>(static_cast<int>(fftOrder));
            window = std::make_unique<juce::dsp::WindowingFunction<float> >(
                fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false);

            inputFifo.resize(fftSize);
            std::fill(inputFifo.begin(), inputFifo.end(), 0.f);
            outputFifo.resize(fftSize);
            std::fill(outputFifo.begin(), outputFifo.end(), 0.f);

            fftData.resize(fftSize << 1);
            std::fill(fftData.begin(), fftData.end(), 0.f);

            corrections.resize(numBins);
            wis1.resize(numBins);
            wis2.resize(numBins);
        }

        void processFrame() {
            const auto *inputPtr = inputFifo.data();
            auto *fftPtr = fftData.data();

            // Copy the input FIFO into the FFT working space in two parts.
            std::memcpy(fftPtr, inputPtr + pos, (fftSize - pos) * sizeof(float));
            if (pos > 0) {
                std::memcpy(fftPtr + fftSize - pos, inputPtr, pos * sizeof(float));
            }

            window->multiplyWithWindowingTable(fftPtr, fftSize);

            fft->performRealOnlyForwardTransform(fftPtr, true);
            processSpectrum();
            fft->performRealOnlyInverseTransform(fftPtr);

            window->multiplyWithWindowingTable(fftPtr, fftSize);

            for (size_t i = 0; i < fftSize; ++i) {
                fftPtr[i] *= windowCorrection;
            }

            for (size_t i = 0; i < pos; ++i) {
                outputFifo[i] += fftData[i + fftSize - pos];
            }
            for (size_t i = 0; i < fftSize - pos; ++i) {
                outputFifo[i + pos] += fftData[i];
            }
        }

        void processSpectrum() {
            if (toUpdate.exchange(false)) {
                update();
            }
            size_t idx = 0;
            for (size_t i = 0; i < corrections.size(); ++i) {
                fftData[idx] *= static_cast<float>(corrections[i].real());
                idx += 1;
                fftData[idx] *= static_cast<float>(corrections[i].imag());
                idx += 1;
            }
        }

        void update() {
            std::fill(corrections.begin(), corrections.end(), 1.f);
            for (size_t idx = 0; idx < filterIndices.size(); ++idx) {
                const auto i = filterIndices[idx];
                idealFs[i].updateResponse(wis1);
                const auto &idealResponse = idealFs[i].getResponse();
                for (size_t j = 0; j < corrections.size(); ++j) {
                    corrections[j] *= idealResponse[j];
                }
                iirFs[i].updateResponse(wis2);
                const auto &iirResponse = iirFs[i].getResponse();
                for (size_t j = 0; j < corrections.size(); ++j) {
                    corrections[j] /= iirResponse[j];
                }
            }
        }
    };
}

#endif //ZLFILTER_PROTOTYPE_CORRECTION_HPP
