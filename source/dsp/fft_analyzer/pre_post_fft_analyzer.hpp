// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

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

        ~PrePostFFTAnalyzer() override {
            if (isThreadRunning()) {
                stopThread(-1);
            }
        }

        void prepare(const juce::dsp::ProcessSpec &spec);

        void prepareBuffer();

        void process(juce::AudioBuffer<FloatType> &pre,
                     juce::AudioBuffer<FloatType> &post,
                     juce::AudioBuffer<FloatType> &side);

        MultipleFFTAnalyzer<FloatType, 3, pointNum> &getMultipleFFT() { return fftAnalyzer; }

        void setON(bool x);

        void setPreON(bool x);

        inline bool getPreON() const { return isPreON.load(); }

        void setPostON(bool x);

        inline bool getPostON() const { return isPostON.load(); }

        void setSideON(bool x);

        inline bool getSideON() const { return isSideON.load(); }

        void updatePaths(juce::Path &prePath_, juce::Path &postPath_, juce::Path &sidePath_,
                         juce::Rectangle<float> bound, float minimumFFTDB);

    private:
        MultipleFFTAnalyzer<FloatType, 3, pointNum> fftAnalyzer;
        std::atomic<bool> isON{false};
        std::atomic<bool> isPreON{true}, isPostON{true}, isSideON{false};
        bool currentON{false}, currentPreON{true}, currentPostON{true}, currentSideON{false};
        std::atomic<float> xx, yy, width, height;
        std::atomic<bool> isBoundReady{false};
        std::atomic<bool> isPathReady{false};
        std::atomic<bool> toReset{false};

        void run() override;

        void handleAsyncUpdate() override;
    };
} // zlFFT

#endif //ZLFFT_PRE_POST_FFT_ANALYZER_HPP
