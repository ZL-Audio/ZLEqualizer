// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "conflict_analyzer.hpp"

namespace zldsp::analyzer {
    template<typename FloatType>
    ConflictAnalyzer<FloatType>::ConflictAnalyzer(const size_t fft_order)
        : Thread("conflict_analyzer"),
          sync_analyzer_(fft_order) {
        sync_analyzer_.setDecayRate(0, 0.985f);
        sync_analyzer_.setDecayRate(1, 0.985f);
        sync_analyzer_.setON({true, true});
    }

    template<typename FloatType>
    ConflictAnalyzer<FloatType>::~ConflictAnalyzer() {
        if (isThreadRunning()) {
            stopThread(-1);
        }
    }

    template<typename FloatType>
    void ConflictAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        sync_analyzer_.prepare(spec);
    }

    template<typename FloatType>
    void ConflictAnalyzer<FloatType>::setON(const bool x) {
        sync_analyzer_.reset();
        is_on_.store(x);
        to_reset_.store(true);
    }

    template<typename FloatType>
    void ConflictAnalyzer<FloatType>::prepareBuffer() {
        current_is_on_ = is_on_.load();
    }

    template<typename FloatType>
    void ConflictAnalyzer<FloatType>::process(juce::AudioBuffer<FloatType> &pre, juce::AudioBuffer<FloatType> &post) {
        if (current_is_on_) {
            sync_analyzer_.process({pre, post});
        }
    }

    template<typename FloatType>
    void ConflictAnalyzer<FloatType>::run() {
        juce::ScopedNoDenormals noDenormals;
        while (!threadShouldExit()) {
            sync_analyzer_.run();
            const auto &mainDB = sync_analyzer_.getInterplotDBs(0);
            const auto &refDB = sync_analyzer_.getInterplotDBs(1);
            const auto mainM = std::reduce(mainDB.begin(), mainDB.end()) / static_cast<float>(mainDB.size());
            const auto refM = std::reduce(refDB.begin(), refDB.end()) / static_cast<float>(refDB.size());
            const auto threshold = juce::jmin(static_cast<float>(strength_.load()) * (mainM + refM), 0.f);

            if (to_reset_.exchange(false)) {
                std::fill(conflicts_.begin(), conflicts_.end(), 0.f);
            }
            for (size_t i = 0; i < conflicts_.size(); ++i) {
                const auto fftIdx = 4 * i;
                const auto dB1 = (mainDB[fftIdx] + mainDB[fftIdx + 1] + mainDB[fftIdx + 2] + mainDB[fftIdx + 3]) *
                                 .25f;
                const auto dB2 = (refDB[fftIdx] + refDB[fftIdx + 1] + refDB[fftIdx + 2] + refDB[fftIdx + 3]) * .25f;
                const auto dBMin = juce::jmin(dB1, dB2, 0.001f);
                conflicts_[i] = juce::jmax(conflicts_[i] * .98f,
                                          (dBMin - threshold) / (0.001f - threshold));
            }
            for (size_t i = 1; i < conflicts_.size() - 1; ++i) {
                conflicts_[i] = conflicts_[i] * .75f + (conflicts_[i - 1] + conflicts_[i + 1]) * .125f;
            }

            // calculate the conflict portion
            const auto scale = static_cast<float>(conflict_scale_.load());
            for (size_t i = 0; i < conflicts_.size(); ++i) {
                conflicts_p_[i] = conflicts_[i] * scale;
                if (conflicts_p_[i].load() >= 0.01) {
                    conflicts_p_[i].store(juce::jmin(.75f, conflicts_p_[i].load()));
                } else {
                    conflicts_p_[i].store(-1.f);
                }
            }
            is_conflict_ready_.store(true);
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
        }
    }

    template<typename FloatType>
    void ConflictAnalyzer<FloatType>::updateGradient(juce::ColourGradient &gradient) {
        if (is_conflict_ready_.load()) {
            // calculate gradient
            gradient.point1 = juce::Point<float>(x1_.load(), 0.f);
            gradient.point2 = juce::Point<float>(x2_.load(), 0.f);
            gradient.isRadial = false;
            gradient.clearColours();

            gradient.addColour(0.0,
                               gColour.withMultipliedAlpha(juce::jmax(conflicts_p_.front().load(), 0.f)));
            gradient.addColour(1.0,
                               gColour.withMultipliedAlpha(juce::jmax(conflicts_p_.back().load(), 0.f)));
            for (size_t i = 1; i < conflicts_p_.size() - 1; ++i) {
                if (conflicts_p_[i + 1] > 0 || conflicts_p_[i - 1] > 0) {
                    const auto p = (static_cast<double>(i) + 0.5) / static_cast<double>(conflicts_p_.size());
                    const auto rectColour = gColour.withMultipliedAlpha(juce::jmax(conflicts_p_[i].load(), 0.f));
                    gradient.addColour(p, rectColour);
                }
            }
            is_conflict_ready_.store(false);
        }
        triggerAsyncUpdate();
    }

    template<typename FloatType>
    void ConflictAnalyzer<FloatType>::handleAsyncUpdate() {
        notify();
    }

    template
    class ConflictAnalyzer<float>;

    template
    class ConflictAnalyzer<double>;
} // zldsp::fft
