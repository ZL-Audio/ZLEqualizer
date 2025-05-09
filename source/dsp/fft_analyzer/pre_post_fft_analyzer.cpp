// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "pre_post_fft_analyzer.hpp"

namespace zldsp::analyzer {
    template<typename FloatType>
    PrePostFFTAnalyzer<FloatType>::PrePostFFTAnalyzer(const size_t fft_order)
        : Thread("pre_post_analyzer"),
          fft_analyzer_(fft_order) {
        fft_analyzer_.setON({is_pre_on_.load(), is_post_on_.load(), is_side_on_.load()});
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        fft_analyzer_.prepare(spec);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::prepareBuffer() {
        current_on_ = is_on_.load();
        if (current_on_) {
            current_pre_on_ = is_pre_on_.load();
            current_post_on_ = is_post_on_.load();
            current_side_on_ = is_side_on_.load();
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::process(juce::AudioBuffer<FloatType> &pre,
                                                juce::AudioBuffer<FloatType> &post,
                                                juce::AudioBuffer<FloatType> &side) {
        if (to_reset_.exchange(false)) {
            fft_analyzer_.reset();
        }
        if (current_on_) {
            fft_analyzer_.process({pre, post, side});
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setON(const bool x) {
        is_on_.store(x);
        if (!x) {
            triggerAsyncUpdate();
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setPreON(const bool x) {
        is_pre_on_.store(x);
        fft_analyzer_.setON({is_pre_on_.load(), is_post_on_.load(), is_side_on_.load()});
        to_reset_.store(true);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setPostON(const bool x) {
        is_post_on_.store(x);
        fft_analyzer_.setON({is_pre_on_.load(), is_post_on_.load(), is_side_on_.load()});
        to_reset_.store(true);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setSideON(const bool x) {
        is_side_on_.store(x);
        fft_analyzer_.setON({is_pre_on_.load(), is_post_on_.load(), is_side_on_.load()});
        to_reset_.store(true);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::run() {
        juce::ScopedNoDenormals no_denormals;
        while (!threadShouldExit()) {
            fft_analyzer_.run();
            is_path_ready_.store(true);
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::handleAsyncUpdate() {
        const auto x = is_on_.load();
        if (x && !isThreadRunning()) {
            startThread(juce::Thread::Priority::low);
        } else if (!x && isThreadRunning()) {
            stopThread(-1);
        } else if (x) {
            notify();
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::updatePaths(
        juce::Path &pre_path, juce::Path &post_path, juce::Path &side_path,
        juce::Rectangle<float> bound, const float minimum_fft_db) {
        if (is_path_ready_.load()) {
            pre_path.clear();
            post_path.clear();
            side_path.clear();
            fft_analyzer_.createPath({pre_path, post_path, side_path}, bound, minimum_fft_db);
            is_path_ready_.store(false);
        }
        triggerAsyncUpdate();
    }

    template
    class PrePostFFTAnalyzer<float>;

    template
    class PrePostFFTAnalyzer<double>;
} // zldsp::fft
