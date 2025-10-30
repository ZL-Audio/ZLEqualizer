// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "controller.hpp"

namespace zlp {
    Controller::Controller(juce::AudioProcessor& processor) :
        p_ref_(processor) {
        not_off_total_.reserve(kBandNum);
        for (auto& v : not_off_indices_) {
            v.reserve(kBandNum);
        }
        correction_on_total_.reserve(kBandNum);
        for (auto& v : correction_on_indices_) {
            v.reserve(kBandNum);
        }
        for (auto& f : side_emptys_) {
            f.setFilterType(zldsp::filter::kBandPass);
        }
    }

    void Controller::prepare(const double sample_rate, const size_t max_num_samples) {
        c_filter_structure_ = kMinimum;

        side_buffers[0].resize(max_num_samples);
        side_buffers[1].resize(max_num_samples);
        side_copy_pointers_.reserve(2);

        for (size_t i = 0; i < kBandNum; ++i) {
            filter_paras_[i] = emptys_[i].getParas();
            side_filter_paras_[i] = side_emptys_[i].getParas();
        }

        for (size_t i = 0; i < kBandNum; ++i) {
            dynamic_side_handlers_[i].prepare(sample_rate);
            tdf_filters_[i].prepare(sample_rate, 2, max_num_samples);
            tdf_filters_[i].getFilter().updateParas(filter_paras_[i]);
            svf_filters_[i].prepare(sample_rate, 2, max_num_samples);
            svf_filters_[i].getFilter().updateParas(filter_paras_[i]);
            parallel_filters_[i].prepare(sample_rate, 2, max_num_samples);
            parallel_filters_[i].getFilter().updateParas(filter_paras_[i]);
            side_filters_[i].prepare(sample_rate, 2, max_num_samples);
            side_filters_[i].updateParas(side_filter_paras_[i]);
            res_ideals_[i].prepare(sample_rate);
            res_tdfs_[i].prepare(sample_rate, 1, 0);
        }
        for (size_t lr = 0; lr < 4; ++lr) {
            match_corrections_[lr].prepare(sample_rate, 1);
            mixed_corrections_[lr].prepare(sample_rate, 1);
            zero_corrections_[lr].prepare(sample_rate, 1);
        }
        match_calculator_.prepare(match_corrections_[0].getNumBin());
        mixed_calculator_.prepare(mixed_corrections_[0].getNumBin());
        zero_calculator_.prepare(zero_corrections_[0].getNumBin());

        hist_unit_decay_ = std::pow(0.9, 1.0 / sample_rate);
        slow_hist_unit_decay_ = std::pow(0.99, 1.0 / sample_rate);

        for (size_t chan = 0; chan < 2; ++chan) {
            pre_main_buffers_[chan].resize(max_num_samples);
            pre_main_pointers_[chan] = pre_main_buffers_[chan].data();
        }
        fft_analyzer_.prepare(sample_rate, {2, 2, 2});

        for (size_t chan = 0; chan < 2; ++chan) {
            solo_buffers_[chan].resize(max_num_samples);
            solo_pointers_[chan] = solo_buffers_[chan].data();
        }
        solo_filter_.prepare(sample_rate, 2, max_num_samples);
        if (c_solo_side_) {
            updateSoloFilter<true>(side_filter_paras_[c_solo_idx_]);
        } else {
            updateSoloFilter<true>(filter_paras_[c_solo_idx_]);
        }

        loudness_matcher_.prepare(sample_rate, 2);
        sgc_gain_.prepare(sample_rate, max_num_samples, 0.5);
        output_gain_.prepare(sample_rate, max_num_samples, 0.5);

        delay_.prepare(sample_rate, max_num_samples, 2, 0.021);
        to_update_delay_.store(true, std::memory_order::release);
        to_update_output_.store(true, std::memory_order::release);
        to_update_.store(true, std::memory_order_release);
    }

    void Controller::prepareBuffer() {
        if (!to_update_.exchange(false, std::memory_order::acquire)) {
            return;
        }
        prepareStatus();
        if (to_update_dynamic_.exchange(false, std::memory_order::acquire)) {
            prepareDynamics();
        }
        if (to_update_lrms_.exchange(false, std::memory_order::acquire)) {
            prepareLRMS();
            to_update_correction_indices_ = true;
            if (c_sgc_on_) {
                updateSGC();
            }
        }
        prepareFilters();
        if (c_correction_enabled_) {
            if (to_update_correction_indices_) {
                prepareCorrectionIndices();
            }
            prepareCorrection();
        }
        prepareDynamicParameters();
        if (to_update_output_.exchange(false, std::memory_order::acquire)) {
            prepareOutput();
        }
    }

    void Controller::prepareStatus() {
        if (c_filter_structure_ != filter_structure_.load(std::memory_order::relaxed)) {
            // cache filter structure
            c_filter_structure_ = filter_structure_.load(std::memory_order::relaxed);
            c_correction_enabled_ =
                (c_filter_structure_ == kMatched) || (c_filter_structure_ == kMixed) || (c_filter_structure_ == kZero);
            if (c_correction_enabled_) {
                // not zero latency, calculate latency later with LRMS info
                to_update_correction_indices_ = true;
                std::fill(res_update_flags_.begin(), res_update_flags_.end(), true);
            } else if (correction_latency_.exchange(0, std::memory_order::relaxed) != 0) {
                // is zero latency, update latency
                triggerAsyncUpdate();
            }
            // force update all filters
            for (size_t i = 0; i < kBandNum; ++i) {
                empty_update_flags_[i].store(true, std::memory_order::relaxed);
            }
            // force reset all filters
            for (size_t i = 0; i < kBandNum; ++i) {
                tdf_filters_[i].reset();
                svf_filters_[i].reset();
                parallel_filters_[i].reset();
            }
            // update lr/ms on flags as they might be changed by corrections
            is_lr_on_ = !not_off_indices_[1].empty() || !not_off_indices_[2].empty();
            is_ms_on_ = !not_off_indices_[3].empty() || !not_off_indices_[4].empty();
            std::fill(to_update_correction_.begin(), to_update_correction_.end(), true);
        } else {
            to_update_correction_indices_ = false;
        }
        if (to_update_status_.exchange(false, std::memory_order::acquire)) {
            // cache total not off indices
            not_off_total_.clear();
            for (size_t i = 0; i < kBandNum; ++i) {
                c_filter_status_[i] = filter_status_[i].load(std::memory_order::relaxed);
                if (c_filter_status_[i] != FilterStatus::kOff) {
                    not_off_total_.emplace_back(i);
                }
            }
            to_update_lrms_.store(true, std::memory_order::release);
        }
        if (to_update_solo_.exchange(false, std::memory_order::acquire)) {
            // update solo status
            const auto solo_whole_idx = solo_whole_idx_.load(std::memory_order::relaxed);
            if (solo_whole_idx == 2 * kBandNum) {
                c_solo_on_ = false;
            } else {
                c_solo_on_ = true;
                solo_filter_.reset();
                if (solo_whole_idx < kBandNum) {
                    c_solo_idx_ = solo_whole_idx;
                    c_solo_side_ = false;
                    updateSoloFilter<true>(filter_paras_[c_solo_idx_]);
                } else {
                    c_solo_idx_ = solo_whole_idx - kBandNum;
                    c_solo_side_ = true;
                    updateSoloFilter<true>(side_filter_paras_[c_solo_idx_]);
                }
            }
        }
    }

    void Controller::prepareFilters() {
        // update not-off filters
        bool to_update_sgc{false};
        for (const size_t& i : not_off_total_) {
            if (empty_update_flags_[i].exchange(false, std::memory_order::acquire)) {
                const auto filter_type = filter_paras_[i].filter_type;
                filter_paras_[i] = emptys_[i].getParas();
                if (c_sgc_on_) {
                    to_update_sgc = true;
                    sgc_values_[i] = zldsp::filter::getGainCompensation(filter_paras_[i]);
                }
                if (c_solo_on_ && !c_solo_side_ && c_solo_idx_ == i) {
                    updateSoloFilter<false>(filter_paras_[i]);
                }
                // if the filter type changes, check whether dynamic should be on
                if (filter_type != filter_paras_[i].filter_type) {
                    prepareOneBandDynamics(i);
                }
                dynamic_side_handlers_[i].setBaseGain(filter_paras_[i].gain);
                if (!c_dynamic_on_[i]) {
                    current_gains_[i].store(filter_paras_[i].gain, std::memory_order::relaxed);
                }
                if (c_filter_structure_ == kSVF) {
                    svf_filters_[i].getFilter().updateParas(filter_paras_[i]);
                } else if (c_filter_structure_ == kParallel) {
                    parallel_filters_[i].getFilter().updateParas(filter_paras_[i]);
                } else {
                    tdf_filters_[i].getFilter().updateParas(filter_paras_[i]);
                }
                res_update_flags_[i] = true;
            }
        }
        if (to_update_sgc) {
            updateSGC();
        }
    }

    void Controller::prepareLRMS() {
        // cache not-off indices and on indices
        for (auto& v : not_off_indices_) {
            v.clear();
        }
        for (const size_t& i : not_off_total_) {
            const auto lr = lrms_[i].load(std::memory_order::relaxed);
            c_lrms_[i] = lr;
            not_off_indices_[static_cast<size_t>(lr)].emplace_back(i);
        }
        is_lr_on_ = !not_off_indices_[1].empty() || !not_off_indices_[2].empty();
        is_ms_on_ = !not_off_indices_[3].empty() || !not_off_indices_[4].empty();
    }

    void Controller::prepareDynamics() {
        // cache dynamic on flags
        for (size_t i = 0; i < kBandNum; ++i) {
            prepareOneBandDynamics(i);
        }
    }

    void Controller::prepareOneBandDynamics(const size_t i) {
        if (dynamic_on_[i].load(std::memory_order::relaxed) && (
            filter_paras_[i].filter_type == zldsp::filter::kPeak ||
            filter_paras_[i].filter_type == zldsp::filter::kLowShelf ||
            filter_paras_[i].filter_type == zldsp::filter::kHighShelf ||
            filter_paras_[i].filter_type == zldsp::filter::kTiltShelf)) {
            // turn on dynamic
            if (c_dynamic_on_[i] != true) {
                c_dynamic_on_[i] = true;
                svf_filters_[i].resetDynamic();
                parallel_filters_[i].resetDynamic();
                tdf_filters_[i].resetDynamic();
                side_filters_[i].reset();
                to_update_correction_indices_ = true;

                side_empty_update_flags_[i].store(true, std::memory_order::relaxed);
                dynamic_th_update_[i].store(true, std::memory_order::relaxed);
                dynamic_ar_update_[i].store(true, std::memory_order::relaxed);
            }
        } else {
            // turn dynamic off and reset filter gain
            if (c_dynamic_on_[i] != false) {
                c_dynamic_on_[i] = false;
                svf_filters_[i].resetDynamic();
                parallel_filters_[i].resetDynamic();
                tdf_filters_[i].resetDynamic();
                current_gains_[i].store(dynamic_side_handlers_[i].getBaseGain(), std::memory_order::relaxed);
                to_update_correction_indices_ = true;
            }
        }
    }

    void Controller::prepareCorrectionIndices() {
        correction_on_total_.clear();
        for (size_t lr = 0; lr < 5; ++lr) {
            correction_on_indices_[lr].clear();
            for (const size_t& i : not_off_indices_[lr]) {
                if (c_filter_status_[i] == FilterStatus::kOn && !c_dynamic_on_[i]) {
                    correction_on_total_.emplace_back(i);
                    correction_on_indices_[lr].emplace_back(i);
                }
            }
        }
        if (correction_on_indices_[3].empty() && correction_on_indices_[4].empty()) {
            for (const size_t& i : correction_on_indices_[0]) {
                correction_on_indices_[1].emplace_back(i);
                correction_on_indices_[2].emplace_back(i);
            }
        } else {
            for (const size_t& i : correction_on_indices_[0]) {
                correction_on_indices_[3].emplace_back(i);
                correction_on_indices_[4].emplace_back(i);
            }
        }
        is_lr_on_ = !not_off_indices_[1].empty() || !not_off_indices_[2].empty() || !correction_on_indices_[1].empty()
            ||
            !correction_on_indices_[2].empty();
        is_ms_on_ = !not_off_indices_[3].empty() || !not_off_indices_[4].empty() || !correction_on_indices_[3].empty()
            ||
            !correction_on_indices_[4].empty();
        // get the latency for one correction
        int unit_latency = 0;
        if (c_filter_structure_ == kMatched) {
            unit_latency = match_corrections_[0].getLatency();
        } else if (c_filter_structure_ == kMixed) {
            unit_latency = mixed_corrections_[0].getLatency();
        } else if (c_filter_structure_ == kZero) {
            unit_latency = zero_corrections_[0].getLatency();
        }
        // calculate total latency and update
        int new_latency = 0;
        if (is_lr_on_) {
            new_latency += unit_latency;
        } else if (is_ms_on_) {
            new_latency += unit_latency;
        }
        if (correction_latency_.exchange(new_latency, std::memory_order::relaxed) != new_latency) {
            triggerAsyncUpdate();
        }
        std::fill(to_update_correction_.begin(), to_update_correction_.end(), true);
    }

    void Controller::prepareCorrection() {
        for (const auto& idx : correction_on_total_) {
            if (res_update_flags_[idx]) {
                res_tdfs_[idx].forceUpdate(filter_paras_[idx]);
                res_ideals_[idx].forceUpdate(filter_paras_[idx]);
            }
        }
        switch (c_filter_structure_) {
        case kMatched: {
            match_calculator_.update(res_tdfs_, res_ideals_, correction_on_total_, res_update_flags_);
            break;
        }
        case kMixed: {
            mixed_calculator_.update(res_tdfs_, res_ideals_, correction_on_total_, res_update_flags_);
            break;
        }
        case kZero: {
            zero_calculator_.update(res_tdfs_, res_ideals_, correction_on_total_, res_update_flags_);
            break;
        }
        case kMinimum:
        case kSVF:
        case kParallel: {
            break;
        }
        }
        for (size_t lr = 0; lr < 4; ++lr) {
            for (const size_t& i : correction_on_indices_[lr]) {
                if (res_update_flags_[i]) {
                    to_update_correction_[lr] = true;
                }
            }
        }
        for (size_t lr = 0; lr < 4; ++lr) {
            for (const size_t& i : correction_on_indices_[lr]) {
                if (res_update_flags_[i]) {
                    res_update_flags_[i] = false;
                }
            }
        }
        for (size_t lr = 0; lr < 4; ++lr) {
            if (to_update_correction_[lr]) {
                to_update_correction_[lr] = false;
                switch (c_filter_structure_) {
                case kMatched: {
                    match_corrections_[lr].updateCorrection(match_calculator_.getCorrections(),
                                                            correction_on_indices_[lr + 1]);
                    break;
                }
                case kMixed: {
                    mixed_corrections_[lr].updateCorrection(mixed_calculator_.getCorrections(),
                                                            correction_on_indices_[lr + 1]);
                    break;
                }
                case kZero: {
                    zero_corrections_[lr].updateCorrection(zero_calculator_.getCorrections(),
                                                           correction_on_indices_[lr + 1]);
                    break;
                }
                case kMinimum:
                case kSVF:
                case kParallel: {
                    break;
                }
                }
            }
        }
    }

    void Controller::prepareDynamicParameters() {
        for (const size_t& i : not_off_total_) {
            if (!dynamic_on_[i]) {
                continue;
            }
            if (side_empty_update_flags_[i].exchange(false, std::memory_order::acquire)) {
                auto side_para = side_emptys_[i].getParas();
                if (side_para.filter_type == zldsp::filter::kLowPass
                    || side_para.filter_type == zldsp::filter::kHighPass) {
                    side_para.q = std::sqrt(2.0) * .5;
                }
                side_filter_paras_[i] = side_para;
                if (c_solo_on_ && c_solo_side_ && c_solo_idx_ == i) {
                    updateSoloFilter<false>(side_para);
                }
                dynamic_side_handlers_[i].setTargetGain(side_para.gain);
                side_para.gain = 0.0;
                side_filters_[i].updateParas(side_para);
            }
            if (dynamic_th_update_[i].exchange(false, std::memory_order::acquire)) {
                if (c_dynamic_th_relative_[i] != dynamic_th_relative_[i].load(std::memory_order::relaxed)) {
                    c_dynamic_th_relative_[i] = dynamic_th_relative_[i].load(std::memory_order::relaxed);
                    if (c_dynamic_th_learn_[i]) {
                        slow_histograms_[i].reset();
                        learned_thresholds_[i].store(-40.0, std::memory_order::relaxed);
                    }
                }
                if (c_dynamic_th_learn_[i] != dynamic_th_learn_[i].load(std::memory_order::relaxed)) {
                    c_dynamic_th_learn_[i] = dynamic_th_learn_[i].load(std::memory_order::relaxed);
                    if (c_dynamic_th_learn_[i]) {
                        histograms_[i].reset();
                        slow_histograms_[i].reset();
                        learned_thresholds_[i].store(-40.0, std::memory_order::relaxed);
                    }
                    hist_results_[1] = 0.0;
                }
                if (c_dynamic_th_learn_[i]) {
                    c_dynamic_threshold_[i] = dynamic_threshold_[i].load(std::memory_order::relaxed) + 40.0;
                } else {
                    c_dynamic_threshold_[i] = dynamic_threshold_[i].load(std::memory_order::relaxed);
                    dynamic_side_handlers_[i].setThreshold<false>(c_dynamic_threshold_[i]);
                }
                dynamic_side_handlers_[i].setKnee(dynamic_knee_[i].load(std::memory_order::relaxed));
            }
            if (dynamic_ar_update_[i].exchange(false, std::memory_order::acquire)) {
                auto& follower{dynamic_side_handlers_[i].getFollower()};
                follower.setAttack<false>(dynamic_attack_[i].load(std::memory_order::relaxed));
                follower.setRelease<true>(dynamic_release_[i].load(std::memory_order::relaxed));
            }
        }
    }

    void Controller::prepareOutput() {
        const auto sgc_on = sgc_on_.load(std::memory_order::relaxed);
        if (c_sgc_on_ != sgc_on) {
            c_sgc_on_ = sgc_on;
            if (c_sgc_on_) {
                for (const size_t& band : not_off_total_) {
                    if (c_filter_status_[band] == kOn) {
                        sgc_values_[band] = zldsp::filter::getGainCompensation(filter_paras_[band]);
                    } else {
                        sgc_values_[band] = 0.0;
                    }
                }
                updateSGC();
            }
        }
        const auto loudness_matcher_on = loudness_matcher_on_.load(std::memory_order_relaxed);
        if (loudness_matcher_on != c_loudness_matcher_on_) {
            c_loudness_matcher_on_ = loudness_matcher_on;
            if (c_loudness_matcher_on_) {
                loudness_matcher_.reset();
            }
        }
        const auto agc_on = agc_on_.load(std::memory_order::relaxed);
        if (c_agc_on_ != agc_on) {
            c_agc_on_ = agc_on;
            updateOutputGain();
        }
        c_phase_flip_on_ = phase_flip_on_.load(std::memory_order_relaxed);
        if (to_update_makeup_.exchange(false, std::memory_order::acquire)) {
            c_makeup_gain_linear_ = makeup_gain_linear_.load(std::memory_order::relaxed);
            updateOutputGain();
        }
        if (to_update_delay_.exchange(false, std::memory_order::acquire)) {
            const auto delay_second = delay_second_.load(std::memory_order::relaxed);
            c_delay_on_ = std::abs(delay_second) > 1e-6;
            delay_.setDelay(c_delay_on_ ? delay_second : 0.);
            delay_latency_.store(delay_.getDelayInSamples(), std::memory_order::relaxed);
            triggerAsyncUpdate();
        }
    }

    template <bool bypass>
    void Controller::process(std::array<double*, 2> main_pointers, std::array<double*, 2> side_pointers,
                             const size_t num_samples) {
        prepareBuffer();
        c_editor_on_ = editor_on_.load(std::memory_order::relaxed);
        if (c_delay_on_) {
            delay_.process(main_pointers, num_samples);
        }
        // copy pre buffer for FFT processing
        if (bypass || c_editor_on_) {
            zldsp::vector::copy(pre_main_pointers_[0], main_pointers[0], num_samples);
            zldsp::vector::copy(pre_main_pointers_[1], main_pointers[1], num_samples);
        }
        // copy solo buffer
        if (c_solo_on_) {
            if (c_solo_side_ || !c_dynamic_on_[c_solo_idx_]) {
                std::memset(solo_pointers_[0], 0, num_samples * sizeof(double));
                std::memset(solo_pointers_[1], 0, num_samples * sizeof(double));
            }
            switch (c_lrms_[c_solo_idx_]) {
            case FilterStereo::kStereo: {
                if (!c_solo_side_) {
                    zldsp::vector::copy(solo_pointers_[0], main_pointers[0], num_samples);
                    zldsp::vector::copy(solo_pointers_[1], main_pointers[1], num_samples);
                    solo_filter_.process(solo_pointers_, num_samples);
                }
                break;
            }
            case FilterStereo::kLeft: {
                std::memset(solo_pointers_[1], 0, num_samples * sizeof(double));
                if (!c_solo_side_) {
                    zldsp::vector::copy(solo_pointers_[0], main_pointers[0], num_samples);
                    solo_filter_.process({&solo_pointers_[0], 1}, num_samples);
                }
                break;
            }
            case FilterStereo::kRight: {
                std::memset(solo_pointers_[0], 0, num_samples * sizeof(double));
                if (!c_solo_side_) {
                    zldsp::vector::copy(solo_pointers_[1], main_pointers[1], num_samples);
                    solo_filter_.process({&solo_pointers_[1], 1}, num_samples);
                }
                break;
            }
            case FilterStereo::kMid: {
                if (!c_solo_side_) {
                    zldsp::vector::copy(solo_pointers_[0], main_pointers[0], num_samples);
                    zldsp::vector::copy(solo_pointers_[1], main_pointers[1], num_samples);
                    zldsp::splitter::InplaceMSSplitter<double>::split(
                        solo_pointers_[0], solo_pointers_[1], num_samples);
                    solo_filter_.process({&solo_pointers_[0], 1}, num_samples);
                }
                std::memset(solo_pointers_[1], 0, num_samples * sizeof(double));
                break;
            }
            case FilterStereo::kSide: {
                if (!c_solo_side_) {
                    zldsp::vector::copy(solo_pointers_[0], main_pointers[0], num_samples);
                    zldsp::vector::copy(solo_pointers_[1], main_pointers[1], num_samples);
                    zldsp::splitter::InplaceMSSplitter<double>::split(
                        solo_pointers_[0], solo_pointers_[1], num_samples);
                    solo_filter_.process({&solo_pointers_[1], 1}, num_samples);
                }
                std::memset(solo_pointers_[0], 0, num_samples * sizeof(double));
                break;
            }
            }
        }
        if (c_loudness_matcher_on_) {
            loudness_matcher_.processPre(main_pointers, num_samples);
        }
        if (c_agc_on_) {
            pre_square_sum_ = 0.0;
            for (size_t chan = 0; chan < 2; chan++) {
                auto v = kfr::make_univector(main_pointers[chan], num_samples);
                pre_square_sum_ += kfr::sumsqr(v);
            }
            pre_square_sum_ = std::clamp(pre_square_sum_ / static_cast<double>(num_samples), 1e-3, 1e3);
        }
        switch (c_filter_structure_) {
        case kMinimum:
        case kMatched:
        case kMixed:
        case kZero: {
            processDynamic(tdf_filters_, main_pointers, side_pointers, num_samples);
            break;
        }
        case kSVF: {
            processDynamic(svf_filters_, main_pointers, side_pointers, num_samples);
            break;
        }
        case kParallel: {
            processParallelPrePost<true>(main_pointers, num_samples);
            processDynamic<std::array<zldsp::filter::DynamicParallel<double, kFilterSize>, kBandNum>, true, true>(
                parallel_filters_, main_pointers, side_pointers, num_samples);
            processParallelPrePost<false>(main_pointers, num_samples);
            processDynamic<std::array<zldsp::filter::DynamicParallel<double, kFilterSize>, kBandNum>, true, false>(
                parallel_filters_, main_pointers, side_pointers, num_samples);
            break;
        }
        }
        if (c_sgc_on_) {
            sgc_gain_.process(main_pointers, num_samples);
        }
        if (c_loudness_matcher_on_) {
            loudness_matcher_.processPost(main_pointers, num_samples);
        }
        if (c_agc_on_) {
            double post_square_sum{0.0};
            for (size_t chan = 0; chan < 2; chan++) {
                auto v = kfr::make_univector(main_pointers[chan], num_samples);
                post_square_sum += kfr::sumsqr(v);
            }
            post_square_sum = std::clamp(post_square_sum / static_cast<double>(num_samples), 1e-3, 1e3);
            c_agc_gain_linear_ = std::sqrt(pre_square_sum_ / post_square_sum) / c_makeup_gain_linear_;
            updateOutputGain();
        }

        output_gain_.process(main_pointers, num_samples);

        if (c_agc_on_) {
            for (size_t chan = 0; chan < 2; chan++) {
                auto v = kfr::make_univector(main_pointers[chan], num_samples);
                v = kfr::clamp(v, -1.0, 1.0);
            }
        }

        if constexpr (bypass) {
            zldsp::vector::copy(main_pointers[0], pre_main_pointers_[0], num_samples);
            zldsp::vector::copy(main_pointers[1], pre_main_pointers_[1], num_samples);
        }

        if (c_editor_on_) {
            fft_analyzer_.process(
                {std::span(pre_main_pointers_), std::span(main_pointers), std::span(side_pointers)},
                num_samples);
            if (c_sgc_on_) {
                displayed_gain_.store(sgc_gain_.getCurrentGainLinear() * output_gain_.getCurrentGainLinear());
            } else {
                displayed_gain_.store(output_gain_.getCurrentGainLinear());
            }
        }

        if (c_solo_on_) {
            switch (c_lrms_[c_solo_idx_]) {
            case FilterStereo::kStereo:
            case FilterStereo::kLeft:
            case FilterStereo::kRight: {
                break;
            }
            case FilterStereo::kMid:
            case FilterStereo::kSide: {
                zldsp::splitter::InplaceMSSplitter<double>::combine(
                    solo_pointers_[0], solo_pointers_[1], num_samples);
                break;
            }
            }
            if constexpr (!bypass) {
                zldsp::vector::copy(main_pointers[0], solo_pointers_[0], num_samples);
                zldsp::vector::copy(main_pointers[1], solo_pointers_[1], num_samples);
            }
        }

        switch (c_filter_structure_) {
        case kMinimum:
        case kSVF:
        case kParallel: {
            break;
        }
        case kMatched: {
            if (c_solo_on_) {
                processCorrections<true>(match_corrections_, main_pointers, num_samples);
            } else {
                processCorrections<bypass>(match_corrections_, main_pointers, num_samples);
            }
            break;
        }
        case kMixed: {
            if (c_solo_on_) {
                processCorrections<true>(mixed_corrections_, main_pointers, num_samples);
            } else {
                processCorrections<bypass>(mixed_corrections_, main_pointers, num_samples);
            }
            break;
        }
        case kZero: {
            if (c_solo_on_) {
                processCorrections<true>(zero_corrections_, main_pointers, num_samples);
            } else {
                processCorrections<bypass>(zero_corrections_, main_pointers, num_samples);
            }
            break;
        }
        }
        if (c_phase_flip_on_) {
            for (size_t chan = 0; chan < 2; chan++) {
                auto v = kfr::make_univector(main_pointers[chan], num_samples);
                v = -v;
            }
        }
    }

    template void Controller::process<true>(std::array<double*, 2>, std::array<double*, 2>, size_t);

    template void Controller::process<false>(std::array<double*, 2>, std::array<double*, 2>, size_t);

    void Controller::handleAsyncUpdate() {
        p_ref_.setLatencySamples(correction_latency_.load(std::memory_order::relaxed)
            + delay_latency_.load(std::memory_order::relaxed));
    }

    template <typename DynamicFilterArrayType, bool should_check_parallel, bool should_be_parallel>
    void Controller::processDynamic(DynamicFilterArrayType& dynamic_filters, std::array<double*, 2> main_pointers,
                                    std::array<double*, 2> side_pointers, const size_t num_samples) {
        processOneChannelDynamic<DynamicFilterArrayType, should_check_parallel, should_be_parallel>(
            dynamic_filters, 0, main_pointers, side_pointers, side_pointers, num_samples);
        if (is_lr_on_) {
            processOneChannelDynamic<DynamicFilterArrayType, should_check_parallel, should_be_parallel>(
                dynamic_filters, 1, {&main_pointers[0], 1}, {&side_pointers[0], 1}, {&side_pointers[1], 1},
                num_samples);
            processOneChannelDynamic<DynamicFilterArrayType, should_check_parallel, should_be_parallel>(
                dynamic_filters, 2, {&main_pointers[1], 1}, {&side_pointers[1], 1}, {&side_pointers[0], 1},
                num_samples);
        }
        if (is_ms_on_) {
            zldsp::splitter::InplaceMSSplitter<double>::split(main_pointers[0], main_pointers[1], num_samples);
            zldsp::splitter::InplaceMSSplitter<double>::split(side_pointers[0], side_pointers[1], num_samples);

            processOneChannelDynamic<DynamicFilterArrayType, should_check_parallel, should_be_parallel>(
                dynamic_filters, 3, {&main_pointers[0], 1}, {&side_pointers[0], 1}, {&side_pointers[1], 1},
                num_samples);
            processOneChannelDynamic<DynamicFilterArrayType, should_check_parallel, should_be_parallel>(
                dynamic_filters, 4, {&main_pointers[1], 1}, {&side_pointers[1], 1}, {&side_pointers[0], 1},
                num_samples);

            zldsp::splitter::InplaceMSSplitter<double>::combine(main_pointers[0], main_pointers[1], num_samples);
            zldsp::splitter::InplaceMSSplitter<double>::combine(side_pointers[0], side_pointers[1], num_samples);
        }
    }

    template <typename DynamicFilterArrayType, bool should_check_parallel, bool should_be_parallel>
    void Controller::processOneChannelDynamic(DynamicFilterArrayType& dynamic_filters, const size_t lrms_idx,
                                              const std::span<double*> main_pointers,
                                              const std::span<double*> side_pointers1,
                                              const std::span<double*> side_pointers2, const size_t num_samples) {
        for (const size_t& i : not_off_indices_[lrms_idx]) {
            if constexpr (should_check_parallel) {
                if (dynamic_filters[i].getShouldBeParallel() != should_be_parallel) {
                    continue;
                }
            }
            if (c_filter_status_[i] == kBypass) {
                if (c_dynamic_on_[i]) {
                    const auto swap = dynamic_swap_[i].load(std::memory_order::relaxed);
                    if (dynamic_bypass_[i].load(std::memory_order::relaxed)) {
                        processOneBandDynamic<true, true, true>(dynamic_filters, i, main_pointers,
                                                                swap ? side_pointers2 : side_pointers1, num_samples);
                    } else {
                        processOneBandDynamic<true, true, false>(dynamic_filters, i, main_pointers,
                                                                 swap ? side_pointers2 : side_pointers1, num_samples);
                    }
                } else {
                    processOneBandDynamic<true, false, false>(dynamic_filters, i, main_pointers, side_pointers1,
                                                              num_samples);
                }
            } else {
                if (c_dynamic_on_[i]) {
                    const auto swap = dynamic_swap_[i].load(std::memory_order::relaxed);
                    if (dynamic_bypass_[i].load(std::memory_order::relaxed)) {
                        processOneBandDynamic<false, true, true>(dynamic_filters, i, main_pointers,
                                                                 swap ? side_pointers2 : side_pointers1, num_samples);
                    } else {
                        processOneBandDynamic<false, true, false>(dynamic_filters, i, main_pointers,
                                                                  swap ? side_pointers2 : side_pointers1, num_samples);
                    }
                } else {
                    processOneBandDynamic<false, false, false>(dynamic_filters, i, main_pointers, side_pointers1,
                                                               num_samples);
                }
            }
        }
    }

    template <bool bypass, bool dynamic_on, bool dynamic_bypass, typename DynamicFilterArrayType>
    void Controller::processOneBandDynamic(DynamicFilterArrayType& dynamic_filters, const size_t i,
                                           const std::span<double*> main_pointers,
                                           const std::span<double*> side_pointers,
                                           const size_t num_samples) {
        if constexpr (dynamic_on) {
            // copy side buffer
            side_copy_pointers_.clear();
            for (size_t chan = 0; chan < side_pointers.size(); ++chan) {
                zldsp::vector::copy(side_buffers[chan].data(), side_pointers[chan], num_samples);
                side_copy_pointers_.emplace_back(side_buffers[chan].data());
            }
            // calculate side total loudness if dynamic relative is on
            double side_total_loudness = 0.0;
            if (c_dynamic_th_relative_[i]) {
                for (size_t chan = 0; chan < side_pointers.size(); ++chan) {
                    auto side_v = kfr::make_univector(side_copy_pointers_[chan], num_samples);
                    side_total_loudness += kfr::sumsqr(side_v) / static_cast<double>(num_samples);
                }
                side_total_loudness = zldsp::chore::squareGainToDecibels(side_total_loudness);
            }
            // process side filter
            side_filters_[i].process(side_copy_pointers_, num_samples);
            // copy side buffer to solo buffer if needed
            if (c_solo_on_ && c_solo_side_ && c_solo_idx_ == i) {
                switch (c_lrms_[i]) {
                case FilterStereo::kStereo: {
                    zldsp::vector::copy(solo_pointers_[0], side_copy_pointers_[0], num_samples);
                    zldsp::vector::copy(solo_pointers_[1], side_copy_pointers_[1], num_samples);
                    break;
                }
                case FilterStereo::kLeft:
                case FilterStereo::kMid: {
                    zldsp::vector::copy(solo_pointers_[0], side_copy_pointers_[0], num_samples);
                    break;
                }
                case FilterStereo::kRight:
                case FilterStereo::kSide: {
                    zldsp::vector::copy(solo_pointers_[1], side_copy_pointers_[0], num_samples);
                    break;
                }
                }
            }
            // calculate side histogram loudness if dynamic learn is on
            if (c_dynamic_th_learn_[i]) {
                double side_current_loudness = 0.0;
                for (size_t chan = 0; chan < side_pointers.size(); ++chan) {
                    auto side_v = kfr::make_univector(side_copy_pointers_[chan], num_samples);
                    side_current_loudness += kfr::sumsqr(side_v) / static_cast<double>(num_samples);
                }
                side_current_loudness = zldsp::chore::squareGainToDecibels(side_current_loudness);
                // update histograms
                slow_histograms_[i].setDecay(std::pow(slow_hist_unit_decay_, static_cast<double>(num_samples)));
                slow_histograms_[i].push(side_current_loudness - side_total_loudness);
                slow_histograms_[i].getPercentiles(hist_percentiles_, hist_target_temp_, hist_results_);
                learned_thresholds_[i].store(hist_results_[1],
                                             std::memory_order::relaxed);
                learned_knees_[i].store(std::max(0.5 * (hist_results_[2] - hist_results_[0]), 2.0),
                                        std::memory_order::relaxed);

                histograms_[i].setDecay(std::pow(hist_unit_decay_, static_cast<double>(num_samples)));
                histograms_[i].push(side_current_loudness);
                histograms_[i].getPercentiles(hist_percentiles_, hist_target_temp_, hist_results_);
                dynamic_side_handlers_[i].setKnee<false>(
                    std::max(0.5 * (hist_results_[2] - hist_results_[0]), 5.0));
            } else if (c_editor_on_) {
                double side_current_loudness = 0.0;
                for (size_t chan = 0; chan < side_pointers.size(); ++chan) {
                    auto side_v = kfr::make_univector(side_copy_pointers_[chan], num_samples);
                    side_current_loudness += kfr::sumsqr(side_v) / static_cast<double>(num_samples);
                }
                side_current_loudness = zldsp::chore::squareGainToDecibels(side_current_loudness);
                dynamic_side_loudness_display_[i].store(side_current_loudness, std::memory_order::relaxed);
            }
            // update actual threshold if required
            if (c_dynamic_th_relative_[i] || c_dynamic_th_learn_[i]) {
                dynamic_side_handlers_[i].setThreshold(
                    hist_results_[1] - side_total_loudness + c_dynamic_threshold_[i]);
            }
        }
        // process the actual filter
        dynamic_filters[i].template processDynamic<bypass, dynamic_on, dynamic_bypass>(
            main_pointers, side_copy_pointers_,
            num_samples);
        if constexpr (dynamic_on) {
            if constexpr (dynamic_bypass) {
                current_gains_[i].store(dynamic_side_handlers_[i].getBaseGain(), std::memory_order::relaxed);
            } else {
                current_gains_[i].store(dynamic_side_handlers_[i].getCurrentGain(), std::memory_order::relaxed);
            }
        }
    }

    template <bool is_pre>
    void Controller::processParallelPrePost(std::span<double*> main_pointers, const size_t num_samples) {
        for (const size_t& i : not_off_indices_[0]) {
            processParallelOneBandPrePost<is_pre>(i, main_pointers, num_samples);
        }
        if (is_lr_on_) {
            std::array<std::array<double*, 1>, 2> main_lr_pointers{{{main_pointers[0]}, {main_pointers[1]}}};
            for (const size_t& i : not_off_indices_[1]) {
                processParallelOneBandPrePost<is_pre>(i, main_lr_pointers[0], num_samples);
            }
            for (const size_t& i : not_off_indices_[2]) {
                processParallelOneBandPrePost<is_pre>(i, main_lr_pointers[1], num_samples);
            }
        }
        if (is_ms_on_) {
            std::array<std::array<double*, 1>, 2> main_ms_pointers{{{main_pointers[0]}, {main_pointers[1]}}};
            zldsp::splitter::InplaceMSSplitter<double>::split(main_pointers[0], main_pointers[1], num_samples);
            for (const size_t& i : not_off_indices_[3]) {
                processParallelOneBandPrePost<is_pre>(i, main_ms_pointers[0], num_samples);
            }
            for (const size_t& i : not_off_indices_[4]) {
                processParallelOneBandPrePost<is_pre>(i, main_ms_pointers[1], num_samples);
            }
            zldsp::splitter::InplaceMSSplitter<double>::combine(main_pointers[0], main_pointers[1], num_samples);
        }
    }

    template <bool is_pre>
    void Controller::processParallelOneBandPrePost(const size_t i,
                                                   std::span<double*> main_pointers, const size_t num_samples) {
        if constexpr (is_pre) {
            if (c_filter_status_[i] == kOn) {
                parallel_filters_[i].processPre<false>(main_pointers, num_samples);
            } else {
                parallel_filters_[i].processPre<true>(main_pointers, num_samples);
            }
        } else {
            parallel_filters_[i].processPost<false>(main_pointers, num_samples);
        }
    }

    template <bool bypass, typename CorrectionArrayType>
    void Controller::processCorrections(CorrectionArrayType& corrections, std::span<double*> main_pointers,
                                        size_t num_samples) {
        if (!correction_on_indices_[1].empty() || !correction_on_indices_[2].empty()) {
            std::array<std::array<double*, 1>, 2> main_lr_pointers{{{main_pointers[0]}, {main_pointers[1]}}};
            corrections[0].template process<bypass>(main_lr_pointers[0], num_samples);
            corrections[1].template process<bypass>(main_lr_pointers[1], num_samples);
        }

        if (!correction_on_indices_[3].empty() || !correction_on_indices_[4].empty()) {
            std::array<std::array<double*, 1>, 2> main_ms_pointers{{{main_pointers[0]}, {main_pointers[1]}}};

            zldsp::splitter::InplaceMSSplitter<double>::split(main_pointers[0], main_pointers[1], num_samples);

            corrections[2].template process<bypass>(main_ms_pointers[0], num_samples);
            corrections[3].template process<bypass>(main_ms_pointers[1], num_samples);

            zldsp::splitter::InplaceMSSplitter<double>::combine(main_pointers[0], main_pointers[1], num_samples);
        }
    }

    template <bool force>
    void Controller::updateSoloFilter(zldsp::filter::FilterParameters paras) {
        switch (paras.filter_type) {
        case zldsp::filter::kPeak:
        case zldsp::filter::kNotch:
        case zldsp::filter::kBandPass:
        case zldsp::filter::kTiltShelf:
        case zldsp::filter::kBandShelf: {
            paras.filter_type = zldsp::filter::kBandPass;
            break;
        }
        case zldsp::filter::kLowShelf:
        case zldsp::filter::kHighPass: {
            paras.q = std::sqrt(2.0) * 0.5;
            paras.filter_type = c_solo_side_ ? zldsp::filter::kHighPass : zldsp::filter::kLowPass;
            break;
        }
        case zldsp::filter::kHighShelf:
        case zldsp::filter::kLowPass: {
            paras.q = std::sqrt(2.0) * 0.5;
            paras.filter_type = c_solo_side_ ? zldsp::filter::kLowPass : zldsp::filter::kHighPass;
            break;
        }
        }
        if constexpr (force) {
            solo_filter_.forceUpdate(paras);
        } else {
            solo_filter_.updateParas(paras);
        }
    }

    void Controller::updateSGC() {
        double sgc_gain_db{0.0};
        for (const size_t& band : not_off_total_) {
            if (c_filter_status_[band] == kOn) {
                switch (c_lrms_[band]) {
                case kStereo: {
                    sgc_gain_db += sgc_values_[band];
                    break;
                }
                case kLeft:
                case kRight: {
                    sgc_gain_db += 0.5 * sgc_values_[band];
                }
                case kMid: {
                    sgc_gain_db += 0.9 * sgc_values_[band];
                }
                case kSide: {
                    sgc_gain_db += 0.1 * sgc_values_[band];
                }
                }
            }
        }
        c_sgc_gain_linear_ = zldsp::chore::decibelsToGain(sgc_gain_db);
        sgc_gain_.setGainLinear(c_sgc_gain_linear_);
    }

    void Controller::updateOutputGain() {
        double total = c_makeup_gain_linear_;
        if (c_agc_on_) {
            total *= c_agc_gain_linear_;
        }
        output_gain_.setGainLinear(std::clamp(total, 1e-3, 1e3));
    }
}
