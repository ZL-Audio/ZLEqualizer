// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "multiple_fft_analyzer.hpp"

namespace zldsp::analyzer {
    /**
     * a conflict fft analyzer
     * @tparam FloatType
     */
    template<typename FloatType>
    class ConflictAnalyzer final : private juce::Thread, juce::AsyncUpdater {
    public:
        static constexpr size_t kPointNum = 251;

        explicit ConflictAnalyzer(size_t fft_order = 12);

        ~ConflictAnalyzer() override;

        void prepare(const juce::dsp::ProcessSpec &spec);

        void prepareBuffer();

        void start() {
            to_reset_.store(true);
            startThread(juce::Thread::Priority::low);
        }

        void stop() {
            stopThread(-1);
        }

        void setON(bool x);

        bool getON() const { return is_on_.load(); }

        void setStrength(const FloatType x) { strength_.store(x); }

        void setConflictScale(const FloatType x) { conflict_scale_.store(x); }

        void process(juce::AudioBuffer<FloatType> &pre,
                     juce::AudioBuffer<FloatType> &post);

        void setLeftRight(const float left, const float right) {
            x1_.store(left);
            x2_.store(right);
        }

        void updateGradient(juce::ColourGradient &gradient);

        MultipleFFTAnalyzer<FloatType, 2, kPointNum> &getSyncFFT() { return sync_analyzer_; }

    private:
        MultipleFFTAnalyzer<FloatType, 2, kPointNum> sync_analyzer_;
        std::atomic<FloatType> strength_{.375f}, conflict_scale_{1.f};
        std::atomic<bool> is_on_{false}, is_conflict_ready_{false}, to_reset_{false};
        bool current_is_on_{false};

        std::atomic<float> x1_{0.f}, x2_{1.f};
        std::array<float, kPointNum / 4> conflicts_{};
        std::array<std::atomic<float>, kPointNum / 4> conflicts_p_{};

        const juce::Colour gColour = juce::Colours::red;

        void run() override;

        void handleAsyncUpdate() override;
    };
} // zldsp::fft
