// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_SINGLE_FFT_ANALYZER_HPP
#define ZLEqualizer_SINGLE_FFT_ANALYZER_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <boost/math/interpolators/cardinal_cubic_b_spline.hpp>

#include "../iir_filter/single_filter.hpp"
#include "../iir_filter/coeff/design_filter.hpp"

namespace zlFFT {
    template<typename FloatType>
    class SingleFFTAnalyzer final : private juce::Thread {
    public:
        explicit SingleFFTAnalyzer(const std::string &name);

        ~SingleFFTAnalyzer() override;

        void prepare(const juce::dsp::ProcessSpec &spec);

        void setOrder(int fftOrder);

        void process(juce::AudioBuffer<FloatType> &buffer);

        void createPath(juce::Path &path, juce::Rectangle<float> bound);

        inline size_t getFFTSize() const { return fftSize.load(); }

        inline bool getIsAudioReady() const { return isAudioReady.load(); }

        inline bool getIsFFTReady() const { return isFFTReady.load(); }

        void resetDecay() { currentDecay.store(1.f); }

        void nextDecay() { currentDecay.store(currentDecay.load() * decayRate.load()); }

    private:
        std::atomic<size_t> delay = 0;
        std::atomic<bool> isAudioReady = false, isFFTReady = false;
        juce::AudioBuffer<float> audioBuffer, fftBuffer; //, averager;
        size_t audioIndex = 0;

        std::vector<float> smoothedDBs;
        static constexpr size_t preScale = 3;
        std::array<float, zlIIR::frequencies.size() / preScale + 2> preInterplotDBs{};
        std::array<float, zlIIR::frequencies.size()> interplotDBs{};
        std::atomic<float> deltaT, decayRate, currentDecay;

        std::unique_ptr<juce::dsp::FFT> fft;
        std::unique_ptr<juce::dsp::WindowingFunction<float> > window;
        juce::ReadWriteLock fftParaLock;
        juce::CriticalSection ampUpdatedLock;
        std::atomic<size_t> fftSize;

        static constexpr auto minFreq = 20.f, maxFreq = 22000.f, minDB = -72.f;
        std::atomic<float> sampleRate;

        void run() override;

        inline float indexToX(const size_t index, const juce::Rectangle<float> bounds) const {
            const auto portion = (static_cast<float>(index) + .5f) / static_cast<float>(fft->getSize());
            return bounds.getX() +
                   bounds.getWidth() * std::log(sampleRate * portion / minFreq) / std::log(maxFreq / minFreq);
        }

        inline float binToY(float bin, const juce::Rectangle<float> bounds) const {
            constexpr float infinity = -72.0f;
            const auto db = juce::Decibels::gainToDecibels(bin);
            return bounds.getY() + (db / infinity) * bounds.getHeight();
        }
    };
} // zlFFT

#endif //ZLEqualizer_SINGLE_FFT_ANALYZER_HPP
