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
            side_filters_[i].updateParas(side_emptys_[i].getParas());
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
    }

    void Controller::prepareBuffer() {
        prepareStatus();
        if (to_update_dynamic_.exchange(false, std::memory_order::acquire)) {
            prepareDynamics();
        }
        if (to_update_lrms_.exchange(false, std::memory_order::acquire)) {
            prepareLRMS();
            to_update_correction_indices_ = true;
        }
        prepareFilters();
        if (c_correction_enabled_) {
            if (to_update_correction_indices_) {
                prepareCorrectionIndices();
            }
            prepareCorrection();
        }
        prepareDynamicParameters();
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
            } else if (latency.exchange(0, std::memory_order::relaxed) != 0) {
                // is zero latency, update latency
                triggerAsyncUpdate();
            }
            // force update all filters
            for (size_t i = 0; i < kBandNum; ++i) {
                emptys_[i].getUpdateParaFlag().store(true, std::memory_order::relaxed);
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
    }

    void Controller::prepareFilters() {
        // update not-off filters
        for (const size_t& i : not_off_total_) {
            if (emptys_[i].getToUpdatePara()) {
                const auto filter_type = filter_paras_[i].filter_type;
                filter_paras_[i] = emptys_[i].getParas();
                // if the filter type changes, check whether dynamic should be on
                if (filter_type != filter_paras_[i].filter_type) {
                    prepareOneBandDynamics(i);
                }
                dynamic_side_handlers_[i].setBaseGain(filter_paras_[i].gain);
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
    }

    void Controller::prepareLRMS() {
        // cache not-off indices and on indices
        for (auto& v : not_off_indices_) {
            v.clear();
        }
        for (const size_t& i : not_off_total_) {
            const auto lr = static_cast<size_t>(lrms_[i].load(std::memory_order::relaxed));
            not_off_indices_[lr].emplace_back(i);
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

                side_emptys_[i].getUpdateParaFlag().store(true, std::memory_order::relaxed);
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
        if (latency.exchange(new_latency, std::memory_order::relaxed) != new_latency) {
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
            if (side_emptys_[i].getToUpdatePara()) {
                auto para = side_emptys_[i].getParas();
                dynamic_side_handlers_[i].setTargetGain(para.gain);
                para.gain = 0.0;
                side_filters_[i].updateParas(para);
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
                    dynamic_side_handlers_[i].setThreshold < false > (c_dynamic_threshold_[i]);
                }
                dynamic_side_handlers_[i].setKnee(dynamic_knee_[i].load(std::memory_order::relaxed));
            }
            if (dynamic_ar_update_[i].exchange(false, std::memory_order::acquire)) {
                auto& follower{dynamic_side_handlers_[i].getFollower()};
                follower.setAttack < false > (dynamic_attack_[i].load(std::memory_order::relaxed));
                follower.setRelease < true > (dynamic_release_[i].load(std::memory_order::relaxed));
            }
        }
    }

    template <bool bypass>
    void Controller::process(std::array<double*, 2> main_pointers, std::array<double*, 2> side_pointers,
                             const size_t num_samples) {
        prepareBuffer();
        c_editor_on_ = editor_on_.load(std::memory_order::relaxed);
        if (c_editor_on_) {
            zldsp::vector::copy(pre_main_pointers_[0], main_pointers[0], num_samples);
            zldsp::vector::copy(pre_main_pointers_[1], main_pointers[1], num_samples);
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
            processDynamic(parallel_filters_, main_pointers, side_pointers, num_samples);
            processParallelPost(main_pointers, num_samples);
            break;
        }
        }

        if (c_editor_on_) {
            fft_analyzer_.process(
                {std::span(main_pointers), std::span(pre_main_pointers_), std::span(side_pointers)},
                num_samples);
        }

        switch (c_filter_structure_) {
        case kMinimum:
        case kSVF:
        case kParallel: {
            break;
        }
        case kMatched: {
            processCorrections<bypass>(match_corrections_, main_pointers, num_samples);
            break;
        }
        case kMixed: {
            processCorrections<bypass>(mixed_corrections_, main_pointers, num_samples);
            break;
        }
        case kZero: {
            processCorrections<bypass>(zero_corrections_, main_pointers, num_samples);
            break;
        }
        }
    }

    template void Controller::process<true>(std::array<double*, 2>, std::array<double*, 2>, size_t);

    template void Controller::process<false>(std::array<double*, 2>, std::array<double*, 2>, size_t);

    void Controller::handleAsyncUpdate() {
        p_ref_.setLatencySamples(latency.load(std::memory_order::relaxed));
    }

    template <typename DynamicFilterArrayType>
    void Controller::processDynamic(DynamicFilterArrayType& dynamic_filters, std::array<double*, 2> main_pointers,
                                    std::array<double*, 2> side_pointers, const size_t num_samples) {
        processOneChannelDynamic(dynamic_filters, 0, main_pointers, side_pointers, side_pointers, num_samples);
        if (is_lr_on_) {
            std::array<std::array<double*, 1>, 2> main_lr_pointers{{{main_pointers[0]}, {main_pointers[1]}}};
            std::array<std::array<double*, 1>, 2> side_lr_pointers{{{side_pointers[0]}, {side_pointers[1]}}};

            processOneChannelDynamic(dynamic_filters, 1, main_lr_pointers[0], side_lr_pointers[0], side_lr_pointers[1],
                                     num_samples);
            processOneChannelDynamic(dynamic_filters, 2, main_lr_pointers[1], side_lr_pointers[1], side_lr_pointers[0],
                                     num_samples);
        }
        if (is_ms_on_) {
            std::array<std::array<double*, 1>, 2> main_ms_pointers{{{main_pointers[0]}, {main_pointers[1]}}};
            std::array<std::array<double*, 1>, 2> side_ms_pointers{{{side_pointers[0]}, {side_pointers[1]}}};

            zldsp::splitter::InplaceMSSplitter<double>::split(main_pointers[0], main_pointers[1], num_samples);
            zldsp::splitter::InplaceMSSplitter<double>::split(side_pointers[0], side_pointers[1], num_samples);

            processOneChannelDynamic(dynamic_filters, 3, main_ms_pointers[0], side_ms_pointers[0], side_ms_pointers[1],
                                     num_samples);
            processOneChannelDynamic(dynamic_filters, 4, main_ms_pointers[1], side_ms_pointers[1], side_ms_pointers[0],
                                     num_samples);

            zldsp::splitter::InplaceMSSplitter<double>::combine(main_pointers[0], main_pointers[1], num_samples);
            zldsp::splitter::InplaceMSSplitter<double>::combine(side_pointers[0], side_pointers[1], num_samples);
        }
    }

    template <typename DynamicFilterArrayType>
    void Controller::processOneChannelDynamic(DynamicFilterArrayType& dynamic_filters, const size_t lrms_idx,
                                              const std::span<double*> main_pointers,
                                              const std::span<double*> side_pointers1,
                                              const std::span<double*> side_pointers2, const size_t num_samples) {
        for (const size_t& i : not_off_indices_[lrms_idx]) {
            if (c_filter_status_[i] == kBypass) {
                if (c_dynamic_on_[i]) {
                    const auto swap = dynamic_swap_[i].load(std::memory_order::relaxed);
                    if (dynamic_bypass_[i].load(std::memory_order::relaxed)) {
                        processOneBandDynamic < true, true, true > (dynamic_filters, i, main_pointers,
                            swap ? side_pointers2 : side_pointers1, num_samples);
                    } else {
                        processOneBandDynamic < true, true, false > (dynamic_filters, i, main_pointers,
                            swap ? side_pointers2 : side_pointers1, num_samples);
                    }
                } else {
                    processOneBandDynamic < true, false, false > (dynamic_filters, i, main_pointers, side_pointers1,
                        num_samples);
                }
            } else {
                if (c_dynamic_on_[i]) {
                    const auto swap = dynamic_swap_[i].load(std::memory_order::relaxed);
                    if (dynamic_bypass_[i].load(std::memory_order::relaxed)) {
                        processOneBandDynamic < false, true, true > (dynamic_filters, i, main_pointers,
                            swap ? side_pointers2 : side_pointers1, num_samples);
                    } else {
                        processOneBandDynamic < false, true, false > (dynamic_filters, i, main_pointers,
                            swap ? side_pointers2 : side_pointers1, num_samples);
                    }
                } else {
                    processOneBandDynamic < false, false, false > (dynamic_filters, i, main_pointers, side_pointers1,
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
                dynamic_side_handlers_[i].setKnee < false > (
                    std::max(0.5 * (hist_results_[2] - hist_results_[0]), 5.0));
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
    }

    void Controller::processParallelPost(std::span<double*> main_pointers, size_t num_samples) {
        for (const size_t& i : not_off_indices_[0]) {
            if (c_filter_status_[i] == kOn) {
                parallel_filters_[i].processPost < false > (main_pointers, num_samples);
            }
        }
        if (is_lr_on_) {
            std::array<std::array<double*, 1>, 2> main_lr_pointers{{{main_pointers[0]}, {main_pointers[1]}}};

            for (const size_t& i : not_off_indices_[1]) {
                if (c_filter_status_[i] == kOn) {
                    parallel_filters_[i].processPost < false > (main_lr_pointers[0], num_samples);
                }
            }

            for (const size_t& i : not_off_indices_[2]) {
                if (c_filter_status_[i] == kOn) {
                    parallel_filters_[i].processPost < false > (main_lr_pointers[1], num_samples);
                }
            }
        }
        if (is_ms_on_) {
            std::array<std::array<double*, 1>, 2> main_ms_pointers{{{main_pointers[0]}, {main_pointers[1]}}};

            zldsp::splitter::InplaceMSSplitter<double>::split(main_pointers[0], main_pointers[1], num_samples);

            for (const size_t& i : not_off_indices_[3]) {
                if (c_filter_status_[i] == kOn) {
                    parallel_filters_[i].processPost < false > (main_ms_pointers[0], num_samples);
                }
            }

            for (const size_t& i : not_off_indices_[4]) {
                if (c_filter_status_[i] == kOn) {
                    parallel_filters_[i].processPost < false > (main_ms_pointers[1], num_samples);
                }
            }

            zldsp::splitter::InplaceMSSplitter<double>::combine(main_pointers[0], main_pointers[1], num_samples);
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
} // namespace zlp
