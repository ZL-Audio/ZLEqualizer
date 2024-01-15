// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_PRE_POST_FFT_ANALYZER_HPP
#define ZLEqualizer_PRE_POST_FFT_ANALYZER_HPP

#include "single_fft_analyzer.hpp"

namespace zlFFT {
    template<typename FloatType>
    class PrePostFFTAnalyzer {
    public:
        PrePostFFTAnalyzer();

        void prepare(const juce::dsp::ProcessSpec &spec);

        void pushPreFFTBuffer(juce::AudioBuffer<FloatType> &buffer);

        void pushPostFFTBuffer(juce::AudioBuffer<FloatType> &buffer);

        void setPreDelay(size_t numSamples);

        SingleFFTAnalyzer<FloatType> &getPreFFT() { return preFFT; }

        SingleFFTAnalyzer<FloatType> &getPostFFT() { return postFFT; }

        inline void setON(const bool x) { isON.store(x); }

    private:
        SingleFFTAnalyzer<FloatType> preFFT{"pre_fft"}, postFFT{"post_fft"};
        juce::AudioBuffer<FloatType> preBuffer, postBuffer;
        juce::dsp::DelayLine<FloatType> preDelay;
        std::atomic<bool> isON = false;
        // juce::FileLogger logger{juce::File("/Volumes/Ramdisk/log.txt"), "prepostlog"};
    };
} // zlFFT

#endif //ZLEqualizer_PRE_POST_FFT_ANALYZER_HPP
