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

        zldsp::filter::IIREmpty empty_;
        zldsp::filter::DynamicTDF<double, 1> filter_;

        void handleAsyncUpdate() override {

        }
    };

    extern template void Controller::process<true>(std::array<double *, 2>, std::array<double *, 2>, size_t);
    extern template void Controller::process<false>(std::array<double *, 2>, std::array<double *, 2>, size_t);
}
