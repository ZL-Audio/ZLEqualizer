// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "multiple_fft_analyzer.hpp"

namespace zldsp::analyzer {
    /**
     * a fft analyzer which process pre, post and side audio buffers
     * @tparam FloatType the float type of input audio buffers
     */
    template<typename FloatType>
    class PrePostFFTAnalyzer final : private juce::Thread, juce::AsyncUpdater {
    public:
        static constexpr size_t kPointNum = 251;

        explicit PrePostFFTAnalyzer(size_t fft_order = 12);

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

        MultipleFFTAnalyzer<FloatType, 3, kPointNum> &getMultipleFFT() { return fft_analyzer_; }

        void setON(bool x);

        void setPreON(bool x);

        inline bool getPreON() const { return is_pre_on_.load(); }

        void setPostON(bool x);

        inline bool getPostON() const { return is_post_on_.load(); }

        void setSideON(bool x);

        inline bool getSideON() const { return is_side_on_.load(); }

        void updatePaths(juce::Path &pre_path, juce::Path &post_path, juce::Path &side_path,
                         juce::Rectangle<float> bound, float minimum_fft_db);

    private:
        MultipleFFTAnalyzer<FloatType, 3, kPointNum> fft_analyzer_;
        std::atomic<bool> is_on_{false};
        std::atomic<bool> is_pre_on_{true}, is_post_on_{true}, is_side_on_{false};
        bool current_on_{false}, current_pre_on_{true}, current_post_on_{true}, current_side_on_{false};
        std::atomic<float> xx_, yy_, width_, height_;
        std::atomic<bool> is_bound_ready_{false};
        std::atomic<bool> is_path_ready_{false};
        std::atomic<bool> to_reset_{false};

        void run() override;

        void handleAsyncUpdate() override;
    };
} // zldsp::fft
