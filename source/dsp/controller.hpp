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

#include "dsp_definitions.hpp"
#include "filter/filter.hpp"
#include "splitter/splitter.hpp"
#include "fft_analyzer/fft_analyzer.hpp"
#include "histogram/histogram.hpp"
#include "gain/gain.hpp"
#include "delay/delay.hpp"
#include "phase/phase.hpp"
#include "eq_match/eq_match.hpp"
#include "loudness/loudness.hpp"

namespace zlp {
    template<typename FloatType>
    class Controller final : public juce::AsyncUpdater {
    public:
        static constexpr size_t kFilterSize = 16;

        explicit Controller(juce::AudioProcessor &processor, size_t fft_order = 12);

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<FloatType> &buffer);

        zldsp::filter::DynamicIIR<FloatType, kFilterSize> &getFilter(const size_t idx) { return filters_[idx]; }

        zldsp::filter::Ideal<FloatType, kFilterSize> &getMainIdealFilter(const size_t idx) { return main_ideals_[idx]; }

        zldsp::filter::IIRIdle<FloatType, kFilterSize> &getMainIIRFilter(const size_t idx) { return main_IIRs_[idx]; }

        std::array<zldsp::filter::DynamicIIR<FloatType, kFilterSize>, kBandNUM> &getFilters() { return filters_; }

        void setFilterLRs(lrType::lrTypes x, size_t idx);

        lrType::lrTypes getFilterLRs(const size_t idx) const { return filers_lrs_[idx].load(); }

        void setDynamicON(bool x, size_t idx);

        void handleAsyncUpdate() override;

        void setSolo(size_t idx, bool isSide);

        bool getSolo() const { return use_solo_.load(); }

        bool getSoloIsSide() const { return solo_side_.load(); }

        void clearSolo(size_t idx, bool isSide);

        inline zldsp::filter::IIR<FloatType, kFilterSize> &getSoloFilter() { return solo_filter_; }

        std::tuple<FloatType, FloatType> getSoloFilterParas(zldsp::filter::FilterType fType, FloatType freq,
                                                            FloatType q);

        inline void setSideChain(const bool x) { side_chain_.store(x); }

        void setRelative(size_t idx, bool isRelative);

        void setSideSwap(size_t idx, bool isSwap);

        void setLearningHistON(size_t idx, bool isLearning);

        bool getLearningHistON(const size_t idx) const { return is_hist_on_[idx].load(); }

        zldsp::histogram::AtomicHistogram<FloatType, 80> &getLearningHist(const size_t idx) {
            return atomic_histograms_[idx];
        }

        void setLookAhead(FloatType x);

        void setRMS(FloatType x);

        void setEffectON(const bool x) { is_effect_on_.store(x); }

        zldsp::analyzer::PrePostFFTAnalyzer<FloatType> &getAnalyzer() { return fft_analyzer_; }

        zldsp::analyzer::ConflictAnalyzer<FloatType> &getConflictAnalyzer() { return conflict_analyzer_; }

        zldsp::eq_match::EqMatchAnalyzer<FloatType> &getMatchAnalyzer() { return match_analyzer_; }

        zldsp::gain::SimpleGain<FloatType> &getGainDSP() { return output_gain_; }

        zldsp::gain::AutoGain<FloatType> &getAutoGain() { return auto_gain_; }

        void setZeroLatency(const bool x) {
            is_zero_latency_.store(x);
            triggerAsyncUpdate();
        }

        FloatType getGainCompensation() const {
            FloatType currentGain = output_gain_.getGainDecibels() + auto_gain_.getGainDecibels();
            currentGain += compensation_gains_[0].getGainDecibels() +
                    FloatType(0.5) * (compensation_gains_[1].getGainDecibels() +
                                      compensation_gains_[2].getGainDecibels()) +
                    FloatType(0.95) * compensation_gains_[3].getGainDecibels() +
                    FloatType(0.05) * compensation_gains_[4].getGainDecibels();
            return currentGain;
        }

        void setThreshold(const size_t idx, const FloatType x) {
            thresholds_[idx].store(x);
        }

        FloatType getThreshold(const size_t idx) const {
            return thresholds_[idx].load();
        }

        FloatType getSideLoudness(const size_t idx) const {
            return side_loudness_[idx].load();
        }

        zldsp::phase::PhaseFlip<FloatType> &getPhaseFlipper() { return phase_flipper_; }

        void setFilterStructure(const filterStructure::FilterStructure x) {
            filter_structure_.store(x);
        }

        void setIsActive(const size_t idx, const bool flag) {
            filters_[idx].setActive(flag);
            is_active_[idx].store(flag);
            to_update_lrs_.store(true);
        }

        void updateSgc(const size_t idx) {
            compensations_[idx].setToUpdate();
            to_update_sgc_.store(true);
        }

        void setSgcON(const bool x) {
            is_sgc_on_.store(x);
        }

        void setBypass(const size_t idx, const bool x) {
            is_bypass_[idx].store(x);
            to_update_bypass_.store(true);
        }

        bool getBypass(const size_t idx) const {
            return is_bypass_[idx].load();
        }

        zldsp::filter::Empty<FloatType> &getBaseFilter(const size_t idx) {
            return b_filters_[idx];
        }

        zldsp::filter::Empty<FloatType> &getTargetFilter(const size_t idx) {
            return t_filters_[idx];
        }

        void setEditorOn(const bool x) { is_editor_on_.store(x); }

        void setLoudnessMatcherON(const bool x) {
            is_loudness_matcher_on_.store(x);
        }

        FloatType getLoudnessMatcherDiff() const {
            return loudness_matcher_.getDiff();
        }

    private:
        juce::AudioProcessor &processor_ref_;

        std::atomic<bool> is_editor_on_{false};
        bool c_is_editor_on_{false};

        std::array<zldsp::filter::Empty<FloatType>, kBandNUM> b_filters_, t_filters_;

        std::array<zldsp::filter::DynamicIIR<FloatType, kFilterSize>, kBandNUM> filters_ =
                [&]<size_t... Is>(std::index_sequence<Is...>) {
                    return std::array{
                        zldsp::filter::DynamicIIR<FloatType, kFilterSize>{
                            std::get<Is>(b_filters_), std::get<Is>(t_filters_)
                        }...
                    };
                }(std::make_index_sequence<std::tuple_size_v<decltype(b_filters_)> >());

        std::array<std::atomic<lrType::lrTypes>, kBandNUM> filers_lrs_;
        std::array<lrType::lrTypes, kBandNUM> current_filter_lrs_{};
        std::array<zldsp::container::FixedMaxSizeArray<size_t, kBandNUM>, 5> filter_lr_indices_;
        std::atomic<bool> to_update_lrs_{true};
        bool use_lr_{false}, use_ms_{false};

        zldsp::container::FixedMaxSizeArray<size_t, kBandNUM> dynamic_on_indices_;
        std::atomic<bool> to_update_dynamic_on_{true};

        std::array<zldsp::filter::StaticGainCompensation<FloatType>, kBandNUM> compensations_ =
                [&]<size_t... Is>(std::index_sequence<Is...>) {
                    return std::array{zldsp::filter::StaticGainCompensation<FloatType>{std::get<Is>(b_filters_)}...};
                }(std::make_index_sequence<std::tuple_size_v<decltype(b_filters_)> >());

        std::array<zldsp::gain::SimpleGain<FloatType>, 5> compensation_gains_;
        std::atomic<bool> is_sgc_on_{false}, to_update_sgc_{false};
        bool c_is_sgc_on_{false};

        std::array<std::atomic<bool>, kBandNUM> is_active_{};
        std::array<std::atomic<bool>, kBandNUM> is_bypass_{};
        std::array<bool, kBandNUM> c_is_bypass_{};
        std::atomic<bool> to_update_bypass_;

        std::array<zldsp::filter::IIRIdle<FloatType, kFilterSize>, kBandNUM> main_IIRs_;
        std::array<zldsp::filter::Ideal<FloatType, kFilterSize>, kBandNUM> main_ideals_;

        std::vector<std::complex<FloatType> > prototype_w1_, prototype_w2_;
        std::array<zldsp::filter::PrototypeCorrection<FloatType, kBandNUM, kFilterSize>, 5> prototype_corrections_ =
                [&]<size_t... Is>(std::index_sequence<Is...>) {
                    return std::array{
                        zldsp::filter::PrototypeCorrection<FloatType, kBandNUM, kFilterSize>{
                            main_IIRs_, main_ideals_, std::get<Is>(filter_lr_indices_), c_is_bypass_, prototype_w1_,
                            prototype_w2_
                        }...
                    };
                }(std::make_index_sequence<std::tuple_size_v<decltype(filter_lr_indices_)> >());

        std::vector<std::complex<FloatType> > mixed_w1_, mixed_w2_;
        std::array<zldsp::filter::MixedCorrection<FloatType, kBandNUM, kFilterSize>, 5> mixed_corrections_ =
                [&]<size_t... Is>(std::index_sequence<Is...>) {
                    return std::array{
                        zldsp::filter::MixedCorrection<FloatType, kBandNUM, kFilterSize>{
                            main_IIRs_, main_ideals_, std::get<Is>(filter_lr_indices_), c_is_bypass_, mixed_w1_,
                            mixed_w2_
                        }...
                    };
                }(std::make_index_sequence<std::tuple_size_v<decltype(filter_lr_indices_)> >());

        std::vector<std::complex<FloatType> > linear_w1_;
        std::array<zldsp::filter::FIR<FloatType, kBandNUM, kFilterSize>, 5> linear_filters_ =
                [&]<size_t... Is>(std::index_sequence<Is...>) {
                    return std::array{
                        zldsp::filter::FIR<FloatType, kBandNUM, kFilterSize>{
                            main_ideals_, std::get<Is>(filter_lr_indices_), c_is_bypass_, linear_w1_
                        }...
                    };
                }(std::make_index_sequence<std::tuple_size_v<decltype(filter_lr_indices_)> >());


        std::atomic<int> latency_{0};

        zldsp::splitter::LRSplitter<FloatType> lr_main_splitter_, lr_side_splitter_;
        zldsp::splitter::MSSplitter<FloatType> ms_main_splitter_, ms_side_splitter_;

        std::array<std::atomic<bool>, kBandNUM> dyn_relatives_, side_swaps_;
        std::atomic<bool> to_update_dyn_rel_side_{true};
        std::array<bool, kBandNUM> c_dyn_relatives_{}, c_side_swaps_{};
        std::array<zldsp::compressor::RMSTracker<FloatType>, 5> trackers_;
        std::array<bool, 5> use_trackers_{};
        std::array<FloatType, 5> tracker_baselines_{};
        std::array<std::atomic<FloatType>, kBandNUM> side_loudness_{};

        std::atomic<bool> side_chain_;

        zldsp::filter::IIR<FloatType, kFilterSize> solo_filter_;
        std::atomic<size_t> solo_idx_;
        std::atomic<bool> to_update_solo_{false};
        std::atomic<bool> use_solo_{false}, solo_side_{false};
        bool c_use_solo_{false}, c_solo_side_{false};
        size_t c_solo_idx_{0};

        std::array<zldsp::histogram::Histogram<FloatType, 80>, kBandNUM> histograms_;
        std::array<zldsp::histogram::Histogram<FloatType, 80>, kBandNUM> sub_histograms_;
        std::array<zldsp::histogram::AtomicHistogram<FloatType, 80>, kBandNUM> atomic_histograms_;
        std::array<std::atomic<bool>, kBandNUM> is_hist_on_{};
        std::array<bool, kBandNUM> c_is_hist_on_{};
        std::atomic<bool> to_update_hist_{true};
        std::array<std::atomic<FloatType>, kBandNUM> thresholds_{};

        zldsp::delay::SampleDelay<FloatType> delay_;

        zldsp::gain::SimpleGain<FloatType> output_gain_;

        zldsp::gain::AutoGain<FloatType> auto_gain_;

        std::atomic<bool> is_effect_on_{true};
        bool c_is_effect_on_{true};

        zldsp::analyzer::PrePostFFTAnalyzer<FloatType> fft_analyzer_;
        zldsp::analyzer::ConflictAnalyzer<FloatType> conflict_analyzer_;
        zldsp::eq_match::EqMatchAnalyzer<FloatType> match_analyzer_;
        juce::AudioBuffer<FloatType> dummy_main_buffer_, dummy_side_buffer_;
        zldsp::delay::SampleDelay<FloatType> dummy_main_delay_, dummy_side_delay_;

        std::atomic<double> sample_rate_{48000};

        std::atomic<bool> is_zero_latency_{false};

        zldsp::phase::PhaseFlip<FloatType> phase_flipper_;

        std::atomic<filterStructure::FilterStructure> filter_structure_{filterStructure::kMinimum};
        filterStructure::FilterStructure c_filter_structure_{filterStructure::kMinimum};

        zldsp::loudness::LUFSMatcher<FloatType, 2, true> loudness_matcher_;
        std::atomic<bool> is_loudness_matcher_on_{false};
        bool c_is_loudness_matcher_on_{false};

        void processSubBuffer(juce::AudioBuffer<FloatType> &sub_main_buffer,
                              juce::AudioBuffer<FloatType> &sub_side_buffer);

        template<bool IsBypassed = false>
        void processSubBufferOnOff(juce::AudioBuffer<FloatType> &sub_main_buffer,
                                   juce::AudioBuffer<FloatType> &sub_side_buffer);

        void processSolo(juce::AudioBuffer<FloatType> &sub_main_buffer,
                         juce::AudioBuffer<FloatType> &sub_side_buffer);

        template<bool IsBypassed = false>
        void processDynamic(juce::AudioBuffer<FloatType> &sub_main_buffer,
                            juce::AudioBuffer<FloatType> &sub_side_buffer);

        template<size_t LRIdx>
        void processDynamicLRMSTrackers(juce::AudioBuffer<FloatType> &sub_side_buffer);

        template<bool IsBypassed = false, size_t LRIdx1, size_t LRIdx2>
        void processDynamicLRMS(juce::AudioBuffer<FloatType> &sub_main_buffer,
                                juce::AudioBuffer<FloatType> &sub_side_buffer1,
                                juce::AudioBuffer<FloatType> &sub_side_buffer2);

        template<bool IsBypassed = false>
        void processParallelPost(juce::AudioBuffer<FloatType> &sub_main_buffer,
                                 juce::AudioBuffer<FloatType> &sub_side_buffer);

        template<bool IsBypassed = false>
        void processParallelPostLRMS(size_t lr_idx,
                                     bool should_parallel,
                                     juce::AudioBuffer<FloatType> &sub_main_buffer,
                                     juce::AudioBuffer<FloatType> &sub_side_buffer);

        template<bool IsBypassed = false>
        void processPrototypeCorrection(juce::AudioBuffer<FloatType> &sub_main_buffer);

        template<bool IsBypassed = false>
        void processMixedCorrection(juce::AudioBuffer<FloatType> &sub_main_buffer);

        template<bool IsBypassed = false>
        void processLinear(juce::AudioBuffer<FloatType> &sub_main_buffer);

        void updateLRs();

        void updateDynamicONs();

        void updateTrackersON();

        void updateSgcValues();

        void updateSubBuffer();

        void updateFilterStructure();

        void updateCorrections();

        void updateSolo();

        void updateDynRelSide();

        void updateHistograms();
    };
}
