// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQMATCH_EQ_MATCH_ANALYZER_HPP
#define ZLEQMATCH_EQ_MATCH_ANALYZER_HPP

#include "../fft_analyzer/fft_analyzer.hpp"

namespace zlEqMatch {
    template<typename FloatType>
    class EqMatchAnalyzer final : private juce::Thread, juce::AsyncUpdater {
    public:
        static constexpr size_t pointNum = 251;

        explicit EqMatchAnalyzer(size_t fftOrder = 12);

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<FloatType> &bBuffer, juce::AudioBuffer<FloatType> &tBuffer);

        zlFFT::AverageFFTAnalyzer<FloatType, pointNum> &getAverageFFT() { return fftAnalyzer; }

        void setON(bool x);

    private:
        zlFFT::AverageFFTAnalyzer<FloatType, pointNum> fftAnalyzer;
        std::atomic<bool> isON{false};
        std::atomic<bool> toReset{false};

        void run() override;

        void handleAsyncUpdate() override;
    };
} // zlEqMatch

#endif //ZLEQMATCH_EQ_MATCH_ANALYZER_HPP
