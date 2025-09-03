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

#include "../dsp/vector/vector.hpp"
#include "../dsp/filter/dynamic_filter/dynamic_tdf.hpp"
#include "../dsp/filter/ideal_filter/ideal.hpp"
#include "../dsp/filter/fir_filter/match_correction/match_correction.hpp"
#include "../dsp/filter/fir_filter/match_correction/match_calculator.hpp"
#include "../dsp/filter/fir_filter/mixed_correction/mixed_correction.hpp"
#include "../dsp/filter/fir_filter/mixed_correction/mixed_calculator.hpp"
#include "../dsp/filter/fir_filter/zero_correction/zero_correction.hpp"
#include "../dsp/filter/fir_filter/zero_correction/zero_calculator.hpp"

namespace zlp {
    class Controller final : private juce::AsyncUpdater {
    public:
        static constexpr size_t kAnalyzerPointNum = 251;

        explicit Controller(juce::AudioProcessor &processor);

        void prepare(double sample_rate, size_t max_num_samples);

        template<bool IsBypassed = false>
        void process(std::array<double *, 2> main_pointers,
                     std::array<double *, 2> side_pointers,
                     size_t num_samples);

    private:
        juce::AudioProcessor &p_ref_;

        zldsp::filter::Empty empty_;
        zldsp::filter::DynamicTDF<double, 1> filter_;

        std::array<zldsp::filter::Ideal<float, 1>, 1> ideal_;
        std::array<zldsp::filter::TDF<float, 1>, 1> tdf_;
        std::array<size_t, 1> update_indices{};
        std::array<size_t, 1> on_indices{};

        zldsp::filter::ZeroCalculator<1, 1> calculator_;
        zldsp::filter::ZeroCorrection<double> correction_;

        void handleAsyncUpdate() override {

        }
    };

    extern template void Controller::process<true>(std::array<double *, 2>, std::array<double *, 2>, size_t);
    extern template void Controller::process<false>(std::array<double *, 2>, std::array<double *, 2>, size_t);
}
