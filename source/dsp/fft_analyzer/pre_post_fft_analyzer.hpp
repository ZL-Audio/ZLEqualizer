// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLFFT_PRE_POST_FFT_ANALYZER_HPP
#define ZLFFT_PRE_POST_FFT_ANALYZER_HPP

#include "multiple_fft_analyzer.hpp"
#include "../delay/delay.hpp"

namespace zlFFT {
    /**
     * a fft analyzer which process pre, post and side audio buffers
     * @tparam FloatType the float type of input audio buffers
     */
    template<typename FloatType>
    class PrePostFFTAnalyzer final : private juce::Thread, juce::AsyncUpdater {
    public:
        static constexpr size_t pointNum = 251;

        explicit PrePostFFTAnalyzer(size_t fftOrder = 12);

        void prepare(const juce::dsp::ProcessSpec &spec);

        void pushPreFFTBuffer(juce::AudioBuffer<FloatType> &buffer);

        void pushPostFFTBuffer(juce::AudioBuffer<FloatType> &buffer);

        void pushSideFFTBuffer(juce::AudioBuffer<FloatType> &buffer);

        void process();

        MultipleFFTAnalyzer<FloatType, 3, pointNum> &getMultipleFFT() { return fftAnalyzer; }

        void setON(bool x);

        void setPreON(bool x);

        inline bool getPreON() const { return isPreON.load(); }

        void setPostON(bool x);

        inline bool getPostON() const { return isPostON.load(); }

        void setSideON(bool x);

        inline bool getSideON() const { return isSideON.load(); }

        bool getPathReady() const { return isPathReady.load(); }

        void updatePaths(juce::Path &prePath_, juce::Path &postPath_, juce::Path &sidePath_,
                         juce::Rectangle<float> bound);

        zlDelay::SampleDelay<FloatType> &getPreDelay() { return preDelay; }

        zlDelay::SampleDelay<FloatType> &getSideDelay() { return sideDelay; }

    private:
        MultipleFFTAnalyzer<FloatType, 3, pointNum> fftAnalyzer;
        juce::AudioBuffer<FloatType> preBuffer, postBuffer, sideBuffer;
        std::atomic<bool> isON{false};
        std::atomic<bool> isPreON{true}, isPostON{true}, isSideON{false};
        bool currentON{false}, currentPreON{true}, currentPostON{true}, currentSideON{false};
        std::atomic<float> xx, yy, width, height;
        std::atomic<bool> isBoundReady{false};
        std::atomic<bool> isPathReady{false};
        std::atomic<bool> toReset{false};
        zlDelay::SampleDelay<FloatType> preDelay, sideDelay;

        void run() override;

        void handleAsyncUpdate() override;
    };
} // zlFFT

#endif //ZLFFT_PRE_POST_FFT_ANALYZER_HPP
