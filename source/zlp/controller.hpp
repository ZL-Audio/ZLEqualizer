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

#include "../dsp/filter/empty_filter/empty.hpp"
#include "../dsp/filter/dynamic_filter/dynamic_tdf.hpp"
#include "../dsp/filter/dynamic_filter/dynamic_svf.hpp"
#include "../dsp/filter/dynamic_filter/dynamic_parallel.hpp"

#include "../dsp/filter/fir_filter/match_correction/match_correction.hpp"
#include "../dsp/filter/fir_filter/match_correction/match_calculator.hpp"
#include "../dsp/filter/fir_filter/mixed_correction/mixed_correction.hpp"
#include "../dsp/filter/fir_filter/mixed_correction/mixed_calculator.hpp"
#include "../dsp/filter/fir_filter/zero_correction/zero_correction.hpp"
#include "../dsp/filter/fir_filter/zero_correction/zero_calculator.hpp"

#include "../dsp/fft_analyzer/multiple_fft_analyzer.hpp"
#include "../dsp/splitter/inplace_ms_splitter.hpp"
#include "../dsp/histogram/histogram.hpp"

namespace zlp {
    template <typename T, std::size_t N, typename... Args, std::size_t... I>
    constexpr std::array<T, N> make_array_of_impl(std::index_sequence<I...>, Args&&... args) {
        return {{(static_cast<void>(I), T(std::forward<Args>(args)...))...}};
    }

    template <typename T, std::size_t N, typename... Args>
    constexpr std::array<T, N> make_array_of(Args&&... args) {
        return make_array_of_impl<T, N>(std::make_index_sequence<N>{}, std::forward<Args>(args)...);
    }

    class Controller final : private juce::AsyncUpdater {
    public:
        static constexpr size_t kFilterSize = 16;
        static constexpr size_t kAnalyzerPointNum = 251;

        explicit Controller(juce::AudioProcessor& processor);

        void prepare(double sample_rate, size_t max_num_samples);

        template <bool bypass = false>
        void process(std::array<double*, 2> main_pointers,
                     std::array<double*, 2> side_pointers,
                     size_t num_samples);

        void setFilterStructure(const FilterStructure filter_structure) {
            filter_structure_.store(filter_structure, std::memory_order::relaxed);
        }

        void setFilterStatus(const size_t idx, const FilterStatus filter_status) {
            filter_status_[idx].store(filter_status, std::memory_order::relaxed);
            to_update_status_.store(true, std::memory_order::release);
        }

        void setLRMS(const size_t idx, const FilterStereo filter_stereo) {
            lrms_[idx].store(filter_stereo, std::memory_order::relaxed);
            to_update_lrms_.store(true, std::memory_order::release);
        }

        void setDynamicON(const size_t idx, const bool dynamic_on) {
            dynamic_on_[idx].store(dynamic_on, std::memory_order::relaxed);
            to_update_dynamic_.store(true, std::memory_order::release);
        }

        void setDynamicBypass(const size_t idx, const bool dynamic_bypass) {
            dynamic_bypass_[idx].store(dynamic_bypass, std::memory_order::relaxed);
        }

        void setDynamicRelative(const size_t idx, const bool is_relative) {
            dynamic_th_relative_[idx].store(is_relative, std::memory_order::relaxed);
            dynamic_th_update_[idx].store(true, std::memory_order::release);
        }

        void setDynamicSwap(const size_t idx, const bool is_swap) {
            dynamic_swap_[idx].store(is_swap, std::memory_order::relaxed);
        }

        void setDynamicLearn(const size_t idx, const bool is_learn) {
            dynamic_th_learn_[idx].store(is_learn, std::memory_order::relaxed);
            dynamic_th_update_[idx].store(true, std::memory_order::release);
        }

        void setDynamicThreshold(const size_t idx, const double threshold) {
            dynamic_threshold_[idx].store(threshold, std::memory_order::relaxed);
            dynamic_th_update_[idx].store(true, std::memory_order::release);
        }

        void setDynamicKnee(const size_t idx, const double knee) {
            dynamic_knee_[idx].store(knee, std::memory_order::relaxed);
            dynamic_th_update_[idx].store(true, std::memory_order::release);
        }

        void setDynamicAttack(const size_t idx, const double attack) {
            dynamic_attack_[idx].store(attack, std::memory_order::relaxed);
            dynamic_ar_update_[idx].store(true, std::memory_order::release);
        }

        void setDynamicRelease(const size_t idx, const double release) {
            dynamic_release_[idx].store(release, std::memory_order::relaxed);
            dynamic_ar_update_[idx].store(true, std::memory_order::release);
        }

        std::array<zldsp::filter::Empty, kBandNum>& getEmptyFilters() {
            return emptys_;
        }

        std::array<zldsp::filter::Empty, kBandNum>& getSideEmptyFilters() {
            return side_emptys_;
        }

        void setEditorON(const bool editor_on) {
            editor_on_.store(editor_on, std::memory_order::relaxed);
        }

        zldsp::analyzer::MultipleFFTAnalyzer<double, 3, 251>& getFFTAnalyzer() {
            return fft_analyzer_;
        }

        double getLearnedThreshold(const size_t idx) const {
            return learned_thresholds_[idx].load(std::memory_order::relaxed);
        }

        double getLearnedKnee(const size_t idx) const {
            return learned_knees_[idx].load(std::memory_order::relaxed);
        }

        std::array<std::atomic<bool>, kBandNum>& getEmptyUpdateFlags() {
            return empty_update_flags_;
        }

        std::array<std::atomic<bool>, kBandNum>& getSideEmptyUpdateFlags() {
            return side_empty_update_flags_;
        }

        double getCurrentGain(const size_t idx) const {
            return current_gains_[idx].load(std::memory_order::relaxed);
        }

    private:
        juce::AudioProcessor& p_ref_;
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
        bool is_lr_on_{false}, is_ms_on_{false};
        // not off indices for stereo/l/r/m/s
        std::array<std::vector<size_t>, 5> not_off_indices_{};
        // empty filters for holding atomic parameters
        std::array<zldsp::filter::Empty, kBandNum> emptys_{};
        std::array<std::atomic<bool>, kBandNum> empty_update_flags_{};
        std::array<zldsp::filter::Empty, kBandNum> side_emptys_{};
        std::array<std::atomic<bool>, kBandNum> side_empty_update_flags_{};
        std::array<zldsp::filter::FilterParameters, kBandNum> filter_paras_{};
        // dynamic handlers
        std::array<zldsp::filter::DynamicSideHandler<double>, kBandNum> dynamic_side_handlers_
            = make_array_of<zldsp::filter::DynamicSideHandler<double>, kBandNum>();
        // dynamic TDF filters
        std::array<zldsp::filter::DynamicTDF<double, kFilterSize>, kBandNum> tdf_filters_
            = [&]<size_t... Is>(std::index_sequence<Is...>) {
                return std::array{
                    zldsp::filter::DynamicTDF<double, kFilterSize>{std::get<Is>(dynamic_side_handlers_)}...
                };
            }(std::make_index_sequence<std::tuple_size_v<decltype(dynamic_side_handlers_)>>());
        // dynamic SVF filters
        std::array<zldsp::filter::DynamicSVF<double, kFilterSize>, kBandNum> svf_filters_
            = [&]<size_t... Is>(std::index_sequence<Is...>) {
                return std::array{
                    zldsp::filter::DynamicSVF<double, kFilterSize>{std::get<Is>(dynamic_side_handlers_)}...
                };
            }(std::make_index_sequence<std::tuple_size_v<decltype(dynamic_side_handlers_)>>());
        // dynamic parallel filters
        std::array<zldsp::filter::DynamicParallel<double, kFilterSize>, kBandNum> parallel_filters_
            = [&]<size_t... Is>(std::index_sequence<Is...>) {
                return std::array{
                    zldsp::filter::DynamicParallel<double, kFilterSize>{std::get<Is>(dynamic_side_handlers_)}...
                };
            }(std::make_index_sequence<std::tuple_size_v<decltype(dynamic_side_handlers_)>>());
        // side-buffer
        std::array<std::vector<double>, 2> side_buffers{};
        std::vector<double*> side_copy_pointers_{};
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
        zldsp::fft::KFREngine<float> match_fft_;
        zldsp::filter::MatchCalculator<kBandNum, kFilterSize> match_calculator_;
        std::array<zldsp::filter::MatchCorrection<double>, 4> match_corrections_
            = make_array_of<zldsp::filter::MatchCorrection<double>, 4>(match_fft_);
        // mixed correction
        zldsp::fft::KFREngine<float> mixed_fft_;
        zldsp::filter::MixedCalculator<kBandNum, kFilterSize> mixed_calculator_;
        std::array<zldsp::filter::MixedCorrection<double>, 4> mixed_corrections_
            = make_array_of<zldsp::filter::MixedCorrection<double>, 4>(mixed_fft_);
        // linear phase (zero phase) correction
        zldsp::fft::KFREngine<float> zero_fft_;
        zldsp::filter::ZeroCalculator<kBandNum, kFilterSize> zero_calculator_;
        std::array<zldsp::filter::ZeroCorrection<double>, 4> zero_corrections_
            = make_array_of<zldsp::filter::ZeroCorrection<double>, 4>(zero_fft_);

        // filter dynamic flags
        std::array<std::atomic<bool>, kBandNum> dynamic_on_{};
        std::array<std::atomic<bool>, kBandNum> dynamic_bypass_{};
        std::array<bool, kBandNum> c_dynamic_on_{};
        std::atomic<bool> to_update_dynamic_{false};
        // filter dynamic swap flags
        std::array<std::atomic<bool>, kBandNum> dynamic_swap_{};
        // dynamic related parameters
        std::array<std::atomic<double>, kBandNum> current_gains_{};
        std::array<std::atomic<bool>, kBandNum> dynamic_th_update_{};
        std::array<std::atomic<bool>, kBandNum> dynamic_th_relative_{};
        std::array<bool, kBandNum> c_dynamic_th_relative_{};
        std::array<std::atomic<bool>, kBandNum> dynamic_th_learn_{};
        std::array<bool, kBandNum> c_dynamic_th_learn_{};
        std::array<std::atomic<double>, kBandNum> dynamic_threshold_{};
        std::array<double, kBandNum> c_dynamic_threshold_{};
        std::array<std::atomic<double>, kBandNum> dynamic_knee_{};
        std::array<std::atomic<bool>, kBandNum> dynamic_ar_update_{};
        std::array<std::atomic<double>, kBandNum> dynamic_attack_{};
        std::array<std::atomic<double>, kBandNum> dynamic_release_{};
        // histogram for dynamic auto-threshold
        double hist_unit_decay_{1.0}, slow_hist_unit_decay_{1.0};
        std::array<double, 3> hist_percentiles_{0.1, 0.5, 0.9};
        std::array<double, 3> hist_target_temp_{};
        std::array<double, 3> hist_results_{};
        std::array<zldsp::histogram::Histogram<double>, kBandNum> histograms_
            = make_array_of<zldsp::histogram::Histogram<double>, kBandNum>(-80.0, 0.0, static_cast<size_t>(80));
        std::array<zldsp::histogram::Histogram<double>, kBandNum> slow_histograms_
            = make_array_of<zldsp::histogram::Histogram<double>, kBandNum>(-80.0, 0.0, static_cast<size_t>(80));
        std::array<std::atomic<double>, kBandNum> learned_thresholds_{};
        std::array<std::atomic<double>, kBandNum> learned_knees_{};
        // array to hold dynamic gains
        std::array<std::atomic<double>, kBandNum> dynamic_gain_display_{};

        std::array<std::vector<double>, 2> pre_main_buffers_{};
        std::array<double*, 2> pre_main_pointers_{};
        std::atomic<bool> editor_on_{false};
        bool c_editor_on_{false};
        zldsp::analyzer::MultipleFFTAnalyzer<double, 3, 251> fft_analyzer_;

        void prepareBuffer();

        void prepareStatus();

        void prepareDynamics();

        void prepareOneBandDynamics(size_t i);

        void prepareLRMS();

        void prepareFilters();

        void prepareCorrectionIndices();

        void prepareCorrection();

        void prepareDynamicParameters();

        void handleAsyncUpdate() override;

        template <typename DynamicFilterArrayType>
        void processDynamic(DynamicFilterArrayType& dynamic_filters,
                            std::array<double*, 2> main_pointers,
                            std::array<double*, 2> side_pointers,
                            size_t num_samples);

        template <typename DynamicFilterArrayType>
        void processOneChannelDynamic(DynamicFilterArrayType& dynamic_filters,
                                      size_t lrms_idx,
                                      std::span<double*> main_pointers,
                                      std::span<double*> side_pointers1,
                                      std::span<double*> side_pointers2,
                                      size_t num_samples);

        template <bool bypass = false, bool dynamic_on = false, bool dynamic_bypass = false,
                  typename DynamicFilterArrayType>
        void processOneBandDynamic(DynamicFilterArrayType& dynamic_filters,
                                   size_t i,
                                   std::span<double*> main_pointers,
                                   std::span<double*> side_pointers,
                                   size_t num_samples);

        void processParallelPost(std::span<double*> main_pointers, size_t num_samples);

        template <bool bypass, typename CorrectionArrayType>
        void processCorrections(CorrectionArrayType& corrections, std::span<double*> main_pointers,
                                size_t num_samples);
    };

    extern template void Controller::process<true>(std::array<double*, 2>, std::array<double*, 2>, size_t);

    extern template void Controller::process<false>(std::array<double*, 2>, std::array<double*, 2>, size_t);
}
