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

        void pushSideFFTBuffer(juce::AudioBuffer<FloatType> &buffer);

        void process();

        SingleFFTAnalyzer<FloatType> &getPreFFT() { return preFFT; }

        SingleFFTAnalyzer<FloatType> &getPostFFT() { return postFFT; }

        SingleFFTAnalyzer<FloatType> &getSideFFT() { return sideFFT; }

        void setON(bool x);

        void setPreON(bool x);

        inline bool getPreON() const { return isPreON.load(); }

        void setPostON(bool x);

        inline bool getPostON() const { return isPostON.load(); }

        void setSideON(bool x);

        inline bool getSideON() const { return isSideON.load(); }

        bool isFFTReady();

    private:
        SingleFFTAnalyzer<FloatType> preFFT{"pre_fft"}, postFFT{"post_fft"}, sideFFT{"side_fft"};
        juce::AudioBuffer<FloatType> preBuffer, postBuffer, sideBuffer;
        std::atomic<bool> isON = false;
        std::atomic<bool> isPreON{true}, isPostON{true}, isSideON{false};

        juce::CriticalSection fftOnOffLock;

        void clearAll();
    };
} // zlFFT

#endif //ZLEqualizer_PRE_POST_FFT_ANALYZER_HPP
