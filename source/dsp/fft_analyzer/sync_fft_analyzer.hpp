// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef SYNC_FFT_ANALYZER_HPP
#define SYNC_FFT_ANALYZER_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include <boost/circular_buffer.hpp>

#include "../iir_filter/single_filter.hpp"
#include "../iir_filter/coeff/design_filter.hpp"

namespace zlFFT {
    /**
     * a fft analyzer which make sure that two FFTs are synchronized in time
     * @tparam FloatType
     */
    template<typename FloatType>
    class SyncFFTAnalyzer final {
    public:
        explicit SyncFFTAnalyzer();

        ~SyncFFTAnalyzer();

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * reset
         */
        void reset();

        void setOrder(int fftOrder);

        /**
         * process (copy) the incoming audio
         * @param buffer1
         * @param buffer2
         */
        void process(juce::AudioBuffer<FloatType> &buffer1, juce::AudioBuffer<FloatType> &buffer2);

        /**
         * run the forward FFT
         */
        void run();

        /**
         * create the FFT path within a given bound
         * @param path1
         * @param path2
         * @param bound
         */
        void createPath(juce::Path &path1, juce::Path &path2, juce::Rectangle<float> bound);

        inline size_t getFFTSize() const { return fftSize.load(); }

        void setDecayRate(const size_t idx, const float x) {
            decayRates[idx].store(x);
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
            for (size_t z = 0; z < 2; ++z) {
                const auto x = 1 - (1 - decayRates[z].load()) * extraSpeed.load();
                actualDecayRate[z].store(std::pow(x, 23.4375f / refreshRate.load()));
            }
        }

        std::array<std::atomic<float>, zlIIR::frequencies.size() / 2> &getInterplotDBs(const size_t z) { return interplotDBs[z]; }

        void setON(const std::array<bool, 2> x) {
            isON[0].store(x[0]);
            isON[1].store(x[1]);
        }

    private:
        std::atomic<size_t> delay = 0;

        std::array<std::vector<float>, 2> currentBuffer;
        size_t currentPos{0};

        std::atomic<int> doubleBufferIdx{0};
        enum { BIT_IDX = (1 << 0), BIT_NEWDATA = (1 << 1), BIT_BUSY = (1 << 2) };
        std::array<std::array<juce::AudioBuffer<float>, 2>, 2> audioBuffer;

        juce::AudioBuffer<float> fftBuffer;

        std::array<std::vector<float>, 2> smoothedDBs;
        std::vector<float> smoothedDBX;
        static constexpr size_t preScale = 5;
        std::array<std::array<float, zlIIR::frequencies.size() / preScale + 2>, 2> preInterplotDBs{};
        std::array<std::array<std::atomic<float>, zlIIR::frequencies.size() / 2>, 2> interplotDBs{};
        std::atomic<float> deltaT, decayRate, refreshRate{60}, tiltSlope;
        std::array<std::atomic<float>, 2> decayRates{}, actualDecayRate{};
        std::atomic<float> extraTilt{0.f}, extraSpeed{1.f};

        std::unique_ptr<juce::dsp::FFT> fft;
        std::unique_ptr<juce::dsp::WindowingFunction<float> > window;
        std::atomic<size_t> fftSize;

        static constexpr auto minFreq = 20.f, maxFreq = 22000.f, minDB = -72.f;
        std::atomic<float> sampleRate;
        std::atomic<bool> toClear{false}, toClearFFT{false};
        std::atomic<bool> isPrepared{false};

        inline float indexToX(const size_t index, const juce::Rectangle<float> bounds) const {
            const auto portion = (static_cast<float>(index) + .5f) / static_cast<float>(fftSize.load());
            return bounds.getX() +
                   bounds.getWidth() * std::log(sampleRate * portion / minFreq) / std::log(maxFreq / minFreq);
        }

        inline float binToY(const float bin, const juce::Rectangle<float> bounds) const {
            constexpr float infinity = -72.0f;
            const auto db = juce::Decibels::gainToDecibels(bin, -240.f);
            return bounds.getY() + (db / infinity) * bounds.getHeight();
        }

        std::array<std::atomic<bool>, 2> isON{true, true};
    };
} // zlFFT

#endif //SYNC_FFT_ANALYZER_HPP
