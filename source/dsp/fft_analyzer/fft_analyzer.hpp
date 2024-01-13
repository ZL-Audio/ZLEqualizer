// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_FFT_ANALYZER_HPP
#define ZLEqualizer_FFT_ANALYZER_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlFFT {
    template<typename FloatType>
    class FFTAnalyzer final : private juce::Thread {
    public:
        explicit FFTAnalyzer(const std::string &name);

        ~FFTAnalyzer() override;

        void prepare(const juce::dsp::ProcessSpec &spec);

        void setOrder(int fftOrder);

        void process(juce::AudioBuffer<FloatType> &buffer);

        void createPath(juce::Path &path, juce::Rectangle<float> bounds);

        inline int getFFTSize() const { return fft->getSize(); }

    private:
        std::atomic<bool> isAudioReady = false, isFFTReady = false;
        juce::AudioBuffer<float> audioBuffer;
        size_t audioIndex = 0;
        // std::vector<FloatType> fftAmplitudes;
        std::vector<juce::LinearSmoothedValue<float> > smoothedDBs;

        std::unique_ptr<juce::dsp::FFT> fft;
        std::unique_ptr<juce::dsp::WindowingFunction<float> > window;
        juce::ReadWriteLock fftParaLock;
        juce::CriticalSection ampUpdatedLock;

        static constexpr auto minFreq = 20.f, maxFreq = 22000.f, minDB = -60.f;
        std::atomic<float> sampleRate;

        void run() override;

        inline float indexToX(const float index, const juce::Rectangle<float> bounds) const {
            return bounds.getX() +
                   bounds.getWidth() * std::log(sampleRate * (index + 0.5f) / minFreq) / std::log(maxFreq / minFreq);
        }

        inline static float binToY(const float bin, const juce::Rectangle<float> bounds) {
            return juce::jmap(bin,
                              minDB, 0.0f,
                              bounds.getBottom(), bounds.getY());
        }
    };
} // zlFFT

#endif //ZLEqualizer_FFT_ANALYZER_HPP
