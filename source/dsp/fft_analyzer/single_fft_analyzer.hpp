// Copyright (C) 2024 - zsliu98
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

#include <boost/circular_buffer.hpp>

#include "../iir_filter/single_filter.hpp"
#include "../iir_filter/coeff/design_filter.hpp"

namespace zlFFT {
    /**
     * a fft analyzer
     * @tparam FloatType
     */
    template<typename FloatType>
    class SingleFFTAnalyzer final {
    public:
        explicit SingleFFTAnalyzer();

        ~SingleFFTAnalyzer();

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * reset
         */
        void reset();

        void setOrder(int fftOrder);

        /**
         * process (copy) the income audio
         * when it has enough audio samples, `isAudioReady` will be set to true
         * and it will stop collecting audio samples
         * @param buffer
         */
        void process(juce::AudioBuffer<FloatType> &buffer);

        /**
         * run the forward FFT
         * when it completes the calculation, `isAudioReady` will be set to false
         * and `isFFTReady` will be set to true
         */
        void run();

        /**
         * create the FFT path within a given bound
         * when it completed the calculation, `isFFTReady` will be set to false
         * @param path
         * @param bound
         */
        void createPath(juce::Path &path, juce::Rectangle<float> bound);

        inline size_t getFFTSize() const { return fftSize.load(); }

        void setDecayRate(const float x) {
            decayRate.store(x);
            updateActualDecayRate();
        }

        void setRefreshRate(const float x) {
            refreshRate.store(x);
            updateActualDecayRate();
        }

        void setTiltSlope(const float x) { tiltSlope.store(x); }

        void setExtraTilt(const float x) { extraTilt.store(x); }

        void setExtraSpeed(const float x) {
            extraSpeed.store(x);
            updateActualDecayRate();
        }

        void updateActualDecayRate() {
            const auto x = 1 - (1 - decayRate.load()) * extraSpeed.load();
            actualDecayRate.store(std::pow(x, 23.4375f / refreshRate.load()));
        }

        std::array<std::atomic<float>, zlIIR::frequencies.size() / 2> &getInterplotDBs() { return interplotDBs; }

    private:
        std::atomic<size_t> delay = 0;

        std::vector<float> currentBuffer;
        size_t currentPos;

        std::atomic<int> doubleBufferIdx{0};
        enum { BIT_IDX = (1 << 0), BIT_NEWDATA = (1 << 1), BIT_BUSY = (1 << 2) };
        std::array<juce::AudioBuffer<float>, 2> audioBuffer;

        juce::AudioBuffer<float> fftBuffer;

        std::vector<float> smoothedDBs, smoothedDBX;
        static constexpr size_t preScale = 5;
        std::array<float, zlIIR::frequencies.size() / preScale + 2> preInterplotDBs{};
        std::array<std::atomic<float>, zlIIR::frequencies.size() / 2> interplotDBs{};
        std::atomic<float> deltaT, decayRate, refreshRate{30}, tiltSlope;
        std::atomic<float> actualDecayRate;
        std::atomic<float> extraTilt{0.f}, extraSpeed{1.f};

        std::unique_ptr<juce::dsp::FFT> fft;
        std::unique_ptr<juce::dsp::WindowingFunction<float> > window;
        // juce::ReadWriteLock fftParaLock;
        std::atomic<size_t> fftSize;

        static constexpr auto minFreq = 20.f, maxFreq = 22000.f, minDB = -72.f;
        std::atomic<float> sampleRate;
        std::atomic<bool> toClear{false}, toClearFFT{false};
        std::atomic<bool> isPrepared{false};

        inline float indexToX(const size_t index, const juce::Rectangle<float> bounds) const {
            const auto portion = (static_cast<float>(index) + .5f) / static_cast<float>(fft->getSize());
            return bounds.getX() +
                   bounds.getWidth() * std::log(sampleRate * portion / minFreq) / std::log(maxFreq / minFreq);
        }

        inline float binToY(const float bin, const juce::Rectangle<float> bounds) const {
            constexpr float infinity = -72.0f;
            const auto db = juce::Decibels::gainToDecibels(bin, -240.f);
            return bounds.getY() + (db / infinity) * bounds.getHeight();
        }
    };
} // zlFFT

#endif //ZLEqualizer_SINGLE_FFT_ANALYZER_HPP
