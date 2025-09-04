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

#include "zlp_definitions.hpp"

#include "../dsp/filter/dynamic_filter/dynamic_tdf.hpp"
#include "../dsp/filter/dynamic_filter/dynamic_svf.hpp"
#include "../dsp/filter/dynamic_filter/dynamic_parallel.hpp"

#include "../dsp/filter/fir_filter/match_correction/match_correction.hpp"
#include "../dsp/filter/fir_filter/match_correction/match_calculator.hpp"
#include "../dsp/filter/fir_filter/mixed_correction/mixed_correction.hpp"
#include "../dsp/filter/fir_filter/mixed_correction/mixed_calculator.hpp"
#include "../dsp/filter/fir_filter/zero_correction/zero_correction.hpp"
#include "../dsp/filter/fir_filter/zero_correction/zero_calculator.hpp"

namespace zlp {
    class Controller final : private juce::AsyncUpdater {
    public:
        static constexpr size_t kFilterSize = 16;
        static constexpr size_t kAnalyzerPointNum = 251;

        explicit Controller(juce::AudioProcessor &processor);

        void prepare(double sample_rate, size_t max_num_samples);

        template<bool IsBypassed = false>
        void process(std::array<double *, 2> main_pointers,
                     std::array<double *, 2> side_pointers,
                     size_t num_samples);

    private:
        juce::AudioProcessor &p_ref_;
        // filter structure
        std::atomic<FilterStructure> filter_structure_{};
        FilterStructure c_filter_structure_{zlp::FilterStructure::kMinimum};
        // filter status
        std::array<std::atomic<FilterStatus>, kBandNum> filter_status_{};
        std::array<FilterStatus, kBandNum> c_filter_status_{};
        std::vector<size_t> not_off_total_{};
        std::atomic<bool> to_update_status_{false};
        // filter l/r/m/s
        std::array<std::atomic<FilterStereo>, kBandNum> lrms_{};
        std::atomic<bool> to_update_lrms_{false};
        // not off indices for stereo/l/r/m/s
        std::array<std::vector<size_t>, 5> not_off_indices_{};
        // filter dynamic flags
        std::array<std::atomic<bool>, kBandNum> dynamic_on_{};
        std::array<bool, kBandNum> c_dynamic_on_{};
        std::atomic<bool> to_update_dynamic_{false};
        // empty filters for holding atomic parameters
        std::array<zldsp::filter::Empty, kBandNum> emptys_{};
        std::array<zldsp::filter::Empty, kBandNum> side_emptys_{};
        std::array<zldsp::filter::FilterParameters, kBandNum> filter_paras_{};
        // dynamic TDF filters
        std::array<zldsp::filter::DynamicTDF<double, kFilterSize>, kBandNum> tdf_filters_{};
        // dynamic SVF filters
        std::array<zldsp::filter::DynamicSVF<double, kFilterSize>, kBandNum> svf_filters_{};
        // dynamic parallel filters
        std::array<zldsp::filter::DynamicParallel<double, kFilterSize>, kBandNum> parallel_filters_{};
        // side-chain filters
        std::array<zldsp::filter::TDF<double, kFilterSize / 2>, kBandNum> side_filters_{};
        // solo filter
        zldsp::filter::TDF<double, kFilterSize / 2> solo_filter_;
        // corrections
        bool c_correction_enabled_{false};
        std::array<bool, 4> to_update_correction_{};
        // update indices
        std::array<bool, kBandNum> res_update_flags_{};
        // correction on indices for stereo/l/r/m/s (might duplicate)
        std::atomic<int> latency{0};
        bool to_update_correction_indices_{false};
        std::vector<size_t> correction_on_total_{};
        std::array<std::vector<size_t>, 5> correction_on_indices_{};
        // filters for calculating prototype response and biquad response
        std::array<zldsp::filter::Ideal<float, kFilterSize>, kBandNum> res_ideals_{};
        std::array<zldsp::filter::TDF<float, kFilterSize>, kBandNum> res_tdfs_{};
        // match correction
        zldsp::filter::MatchCalculator<kBandNum, kFilterSize> match_calculator_;
        std::array<zldsp::filter::MatchCorrection<double>, 4> match_corrections_{};
        // mixed correction
        zldsp::filter::MixedCalculator<kBandNum, kFilterSize> mixed_calculator_;
        std::array<zldsp::filter::MixedCorrection<double>, 4> mixed_corrections_{};
        // linear phase (zero phase) correction
        zldsp::filter::ZeroCalculator<kBandNum, kFilterSize> zero_calculator_;
        std::array<zldsp::filter::ZeroCorrection<double>, 4> zero_corrections_{};
        // array to hold dynamic gains
        std::array<std::atomic<double>, kBandNum> dynamic_gain_display_{};

        void prepareBuffer();

        void prepareStatus();

        void prepareDynamics();

        template <FilterStructure structure>
        void prepareOneBandDynamics(size_t i);

        void prepareLRMS();

        void prepareFilters();

        void prepareCorrectionIndices();

        void prepareCorrection();

        void prepareSideFilters();

        void handleAsyncUpdate() override;
    };

    extern template void Controller::process<true>(std::array<double *, 2>, std::array<double *, 2>, size_t);

    extern template void Controller::process<false>(std::array<double *, 2>, std::array<double *, 2>, size_t);
}
