// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_LOUDNESS_LUFS_METER_HPP
#define ZL_LOUDNESS_LUFS_METER_HPP

#include "k_weighting_filter.hpp"

namespace zlLoudness {
    template<typename FloatType, size_t MaxChannels = 2, bool UseLowPass = false>
    class LUFSMeter {
    public:
        LUFSMeter() {
            for (size_t i = 0; i < MaxChannels; ++i) {
                weights[i] = (i == 4 || i == 5) ? FloatType(1.41) : FloatType(1);
            }
        }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            kWeightingFilter.prepare(spec);
            maxIdx = static_cast<int>(spec.sampleRate * 0.1);
            meanMul = static_cast<FloatType>(2.5 / spec.sampleRate);
            smallBuffer.setSize(static_cast<int>(spec.numChannels), maxIdx);
            reset();
        }

        void reset() {
            kWeightingFilter.reset();
            std::fill(histogram.begin(), histogram.end(), FloatType(0));
            std::fill(histogramSums.begin(), histogramSums.end(), FloatType(0));
            currentIdx = 0;
            smallBuffer.clear();
            readyCount = 0;
        }

        void process(juce::AudioBuffer<FloatType> &buffer) {
            process(juce::dsp::AudioBlock<FloatType>(buffer));
        }

        void process(juce::dsp::AudioBlock<FloatType> block) {
            const auto numTotal = static_cast<int>(block.getNumSamples());
            int startIdx = 0;
            juce::dsp::AudioBlock<FloatType> smallBlock(smallBuffer);
            while (numTotal - startIdx >= maxIdx - currentIdx) {
                // now we get a full 100ms small block
                const auto subBlock = block.getSubBlock(static_cast<size_t>(startIdx),
                                                        static_cast<size_t>(maxIdx - currentIdx));
                auto smallSubBlock = smallBlock.getSubBlock(static_cast<size_t>(currentIdx),
                                                            static_cast<size_t>(maxIdx - currentIdx));
                smallSubBlock.copyFrom(subBlock);
                startIdx += maxIdx - currentIdx;
                currentIdx = 0;
                update();
            }
            if (numTotal - startIdx > 0) {
                const auto subBlock = block.getSubBlock(static_cast<size_t>(startIdx),
                static_cast<size_t>(numTotal - startIdx));
                auto smallSubBlock = smallBlock.getSubBlock(static_cast<size_t>(currentIdx),
                static_cast<size_t>(numTotal - startIdx));
                smallSubBlock.copyFrom(subBlock);
                currentIdx += numTotal - startIdx;
            }
        }

        FloatType getIntegratedLoudness() const {
            const auto totalCount = std::reduce(histogram.begin(), histogram.end(), FloatType(0));
            if (totalCount < FloatType(0.5)) { return FloatType(0); }
            const auto totalSum = std::reduce(histogramSums.begin(), histogramSums.end(), FloatType(0));
            const auto totalLUFS = totalSum / totalCount;
            if (totalLUFS <= FloatType(-60)) {
                return totalLUFS;
            } else {
                const auto endIdx = static_cast<size_t>(std::round(-(totalLUFS - FloatType(10)) * FloatType(10)));
                const auto subCount = std::reduce(histogram.begin(), histogram.begin() + endIdx, FloatType(0));
                const auto subSum = std::reduce(histogramSums.begin(), histogramSums.begin() + endIdx, FloatType(0));
                return subSum / subCount;
            }
        }

    private:
        KWeightingFilter<FloatType, UseLowPass> kWeightingFilter;
        juce::AudioBuffer<FloatType> smallBuffer;
        int currentIdx{0}, maxIdx{0};
        int readyCount{0};
        FloatType meanMul{1};
        std::array<FloatType, 4> sumSquares{};

        std::array<FloatType, 701> histogram{};
        std::array<FloatType, 701> histogramSums{};
        std::array<FloatType, MaxChannels> weights;

        void update() {
            // perform K-weighting filtering
            kWeightingFilter.process(smallBuffer);
            // calculate the sum square of the small block
            FloatType sumSquare = 0;
            for (int channel = 0; channel < smallBuffer.getNumChannels(); ++channel) {
                const auto readerPointer = smallBuffer.getReadPointer(channel);
                FloatType channelSumSquare = 0;
                for (int i = 0; i < smallBuffer.getNumSamples(); ++i) {
                    const auto sample = *(readerPointer + i);
                    channelSumSquare += sample * sample;
                }
                sumSquare += channelSumSquare * weights[static_cast<size_t>(channel)];
            }
            // shift circular sumSquares
            sumSquares[0] = sumSquares[1];
            sumSquares[1] = sumSquares[2];
            sumSquares[2] = sumSquares[3];
            sumSquares[3] = sumSquare;
            if (readyCount < 3) {
                readyCount += 1;
                return;
            }
            // calculate the mean square
            const auto meanSquare = (sumSquares[0] + sumSquares[1] + sumSquares[2] + sumSquares[3]) * meanMul;
            // update histogram
            if (meanSquare >= FloatType(1.1724653045822963e-7)) {
                // if greater than -70 LKFS
                const auto LKFS = std::min(-FloatType(0.691) + FloatType(10) * std::log10(meanSquare), FloatType(0));
                const auto histIdx = static_cast<size_t>(std::round(-LKFS * FloatType(10)));
                histogram[histIdx] += FloatType(1);
                histogramSums[histIdx] += LKFS;
            }
        }
    };
}

#endif //ZL_LOUDNESS_LUFS_METER_HPP
