// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLFILTER_FIR_FILTER_HPP
#define ZLFILTER_FIR_FILTER_HPP

#include <cmath>

#include "../iir_filter/iir_filter.hpp"
#include "../ideal_filter/ideal_filter.hpp"
#include "../../container/array.hpp"

namespace zlFilter {
    /**
     * an FIR which has the magnitude responses of prototype filters and zero phase responses
     * @tparam FloatType the float type of input audio buffer
     * @tparam FilterNum the number of filters
     * @tparam FilterSize the size of each filter
     * @tparam defaultFFTOrder the default FFT order for 44100/48000 Hz input
     */
    template<typename FloatType, size_t FilterNum, size_t FilterSize, size_t defaultFFTOrder = 13>
    class FIR {
    public:
        FIR(std::array<Ideal<FloatType, FilterSize>, FilterNum> &ideal,
            zlContainer::FixedMaxSizeArray<size_t, FilterNum> &indices,
            std::array<bool, FilterNum> &mask,
            std::vector<std::complex<FloatType> > &w1)

            : idealFs(ideal),
              filterIndices(indices), bypassMask(mask),
              wis1(w1) {
        }

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

        void setToUpdate() { toUpdate.store(true); }

        size_t getCorrectionSize() const { return corrections.size(); }

    private:
        std::array<Ideal<FloatType, FilterSize>, FilterNum> &idealFs;
        zlContainer::FixedMaxSizeArray<size_t, FilterNum> &filterIndices;
        std::array<bool, FilterNum> &bypassMask;
        std::atomic<bool> toUpdate{true};

        std::vector<std::complex<FloatType> > idealTotalResponse;

        std::vector<float> corrections{}, dummyCorrections{};
        std::vector<std::complex<FloatType> > &wis1;

        std::unique_ptr<juce::dsp::FFT> fft;
        std::unique_ptr<juce::dsp::WindowingFunction<float> > window;

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
        std::vector<std::vector<float> > inputFIFOs, outputFIFOs;
        // circular FFT working space which contains interleaved complex numbers.
        std::vector<float> fftData;
        size_t fftDataPos = 0;

        std::atomic<int> latency{0};

        void setOrder(const size_t channelNum, const size_t order) {
            fftOrder = order;
            fftSize = static_cast<size_t>(1) << fftOrder;
            numBins = fftSize / 2 + 1;
            hopSize = fftSize / overlap;
            latency.store(static_cast<int>(fftSize));

            fft = std::make_unique<juce::dsp::FFT>(static_cast<int>(fftOrder));
            window = std::make_unique<juce::dsp::WindowingFunction<float> >(
                fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false);

            inputFIFOs.resize(channelNum);
            outputFIFOs.resize(channelNum);
            fftData.resize(fftSize * 2);

            corrections.resize(numBins);
            dummyCorrections.resize(numBins << 1);
            reset();
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
                    window->multiplyWithWindowingTable(fftPtr, fftSize);

                    fft->performRealOnlyForwardTransform(fftPtr, true);
                    processSpectrum();
                    fft->performRealOnlyInverseTransform(fftPtr);

                    window->multiplyWithWindowingTable(fftPtr, fftSize);
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

        void processSpectrum() {
            update();
            juce::FloatVectorOperations::multiply(fftData.data(),
                                                  dummyCorrections.data(),
                                                  dummyCorrections.size());
        }

        void update() {
            // check whether a filter has been updated
            bool needToUpdate{false};
            for (size_t idx = 0; idx < filterIndices.size(); ++idx) {
                const auto i = filterIndices[idx];
                if (!bypassMask[i]) {
                    needToUpdate = needToUpdate || idealFs[i].updateZeroPhaseResponse(wis1);
                }
            }
            // if a filter has been updated or the correction has to be updated
            if (needToUpdate || toUpdate.exchange(false)) {
                bool hasBeenUpdated = false;
                for (size_t idx = 0; idx < filterIndices.size(); ++idx) {
                    const auto i = filterIndices[idx];
                    if (!bypassMask[i]) {
                        const auto &idealResponse = idealFs[i].getResponse();
                        if (!hasBeenUpdated) {
                            for (size_t j = 1; j < corrections.size(); ++j) {
                                corrections[j] = static_cast<float>(idealResponse[j].real());
                            }
                            hasBeenUpdated = true;
                        } else {
                            for (size_t j = 1; j < corrections.size(); ++j) {
                                corrections[j] *= static_cast<float>(idealResponse[j].real());
                            }
                        }
                    }
                }
                if (!hasBeenUpdated) {
                    std::fill(dummyCorrections.begin(), dummyCorrections.end(), 1.f);
                } else {
                    corrections[0] = corrections[1];
                    for (size_t j = 0; j < corrections.size(); ++j) {
                        dummyCorrections[j << 1] = corrections[j];
                        dummyCorrections[(j << 1) + 1] = corrections[j];
                    }
                }
            }
        }
    };
}

#endif //ZLFILTER_FIR_FILTER_HPP
