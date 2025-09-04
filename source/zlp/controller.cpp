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
    Controller::Controller(juce::AudioProcessor &processor)
        : p_ref_(processor) {
        not_off_total_.reserve(kBandNum);
        for (auto &v: not_off_indices_) {
            v.reserve(kBandNum);
        }
        correction_on_total_.reserve(kBandNum);
        for (auto &v: correction_on_indices_) {
            v.reserve(kBandNum);
        }
    }

    void Controller::prepare(double sample_rate, size_t max_num_samples) {
        juce::ignoreUnused(sample_rate, max_num_samples);
        c_filter_structure_ = kMinimum;

        for (size_t i = 0; i < kBandNum; ++i) {
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
    }

    void Controller::prepareBuffer() {
        prepareStatus();
        if (to_update_dynamic_.exchange(false, std::memory_order::relaxed)) {
            prepareDynamics();
        }
        if (to_update_lrms_.exchange(false, std::memory_order::relaxed)) {
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
    }

    void Controller::prepareStatus() {
        if (c_filter_structure_ != filter_structure_.load(std::memory_order::relaxed)) {
            // cache filter structure
            c_filter_structure_ = filter_structure_.load(std::memory_order::relaxed);
            c_correction_enabled_ = (c_filter_structure_ == kMatched)
                                    || (c_filter_structure_ == kMixed)
                                    || (c_filter_structure_ == kLinear);
            if (c_correction_enabled_) {
                // not zero latency, calculate latency later with LRMS info
                to_update_correction_indices_ = true;
                std::fill(res_update_flags_.begin(), res_update_flags_.end(), true);
            } else if (latency.exchange(0, std::memory_order::relaxed) != 0) {
                // is zero latency, update latency
                triggerAsyncUpdate();
            }
            std::fill(to_update_correction_.begin(), to_update_correction_.end(), true);
        } else {
            to_update_correction_indices_ = false;
        }
        if (to_update_status_.exchange(false, std::memory_order::relaxed)) {
            // cache total not off indices
            not_off_total_.clear();
            for (size_t i = 0; i < kBandNum; ++i) {
                c_filter_status_[i] = filter_status_[i].load(std::memory_order::relaxed);
                if (c_filter_status_[i] != FilterStatus::kOff) {
                    not_off_total_.emplace_back(i);
                }
            }
        }
    }

    void Controller::prepareFilters() {
        for (const size_t &i: not_off_total_) {
            if (emptys_[i].getToUpdatePara()) {
                const auto filter_type = filter_paras_[i].filter_type;
                filter_paras_[i] = emptys_[i].getParas();
                // if the filter type changes, check whether dynamic should be on
                if (filter_type != filter_paras_[i].filter_type) {
                    if (c_filter_structure_ == kSVF) {
                        prepareOneBandDynamics<kSVF>(i);
                    } else if (c_filter_structure_ == kParallel) {
                        prepareOneBandDynamics<kParallel>(i);
                    } else {
                        prepareOneBandDynamics<kMinimum>(i);
                    }
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
    }

    void Controller::prepareLRMS() {
        // cache not-off indices and on indices
        for (auto &v: not_off_indices_) {
            v.clear();
        }
        for (const size_t &i: not_off_total_) {
            const auto lr = static_cast<size_t>(lrms_[i].load(std::memory_order::relaxed));
            not_off_indices_[lr].emplace_back(i);
        }
    }

    void Controller::prepareDynamics() {
        // cache dynamic on flags
        for (size_t i = 0; i < kBandNum; ++i) {
            if (c_filter_structure_ == kSVF) {
                prepareOneBandDynamics<kSVF>(i);
            } else if (c_filter_structure_ == kParallel) {
                prepareOneBandDynamics<kParallel>(i);
            } else {
                prepareOneBandDynamics<kMinimum>(i);
            }
        }
    }

    template<FilterStructure structure>
    void Controller::prepareOneBandDynamics(const size_t i) {
        if (dynamic_on_[i].load(std::memory_order::relaxed) &&
            (filter_paras_[i].filter_type == zldsp::filter::kPeak ||
             filter_paras_[i].filter_type == zldsp::filter::kLowShelf ||
             filter_paras_[i].filter_type == zldsp::filter::kHighShelf ||
             filter_paras_[i].filter_type == zldsp::filter::kTiltShelf)) {
            // turn on dynamic
            if (c_dynamic_on_[i] != true) {
                c_dynamic_on_[i] = true;
                if constexpr (structure == kSVF) {
                    svf_filters_[i].setDynamicON(true);
                } else if constexpr (structure == kParallel) {
                    parallel_filters_[i].setDynamicON(true);
                } else {
                    tdf_filters_[i].setDynamicON(true);
                }
                to_update_correction_indices_ = true;
            }
        } else {
            // turn dynamic off and reset filter gain
            if (c_dynamic_on_[i] != false) {
                c_dynamic_on_[i] = false;
                if constexpr (structure == kSVF) {
                    svf_filters_[i].setDynamicON(false);
                    svf_filters_[i].getFilter().updateParas(filter_paras_[i]);
                } else if constexpr (structure == kParallel) {
                    parallel_filters_[i].setDynamicON(false);
                    parallel_filters_[i].getFilter().updateParas(filter_paras_[i]);
                } else {
                    tdf_filters_[i].setDynamicON(false);
                    tdf_filters_[i].getFilter().updateParas(filter_paras_[i]);
                }
                to_update_correction_indices_ = true;
            }
        }
    }

    void Controller::prepareCorrectionIndices() {
        correction_on_total_.clear();
        for (size_t lr = 0; lr < 5; ++lr) {
            correction_on_indices_[lr].clear();
            for (const size_t &i: not_off_indices_[lr]) {
                if (c_filter_status_[i] == FilterStatus::kOn && !c_dynamic_on_[i]) {
                    correction_on_total_.emplace_back(i);
                    correction_on_indices_[lr].emplace_back(i);
                }
            }
        }
        if (correction_on_indices_[3].empty() && correction_on_indices_[4].empty()) {
            for (const size_t &i: correction_on_indices_[0]) {
                correction_on_indices_[1].emplace_back(i);
                correction_on_indices_[2].emplace_back(i);
            }
        } else {
            for (const size_t &i: correction_on_indices_[0]) {
                correction_on_indices_[3].emplace_back(i);
                correction_on_indices_[4].emplace_back(i);
            }
        }
        // get the latency for one correction
        int unit_latency = 0;
        if (c_filter_structure_ == kMatched) {
            unit_latency = match_corrections_[0].getLatency();
        } else if (c_filter_structure_ == kMixed) {
            unit_latency = mixed_corrections_[0].getLatency();
        } else if (c_filter_structure_ == kLinear) {
            unit_latency = zero_corrections_[0].getLatency();
        }
        // calculate total latency and update
        int new_latency = 2 * unit_latency;
        if (correction_on_indices_[0].empty() && correction_on_indices_[1].empty()) {
            new_latency -= unit_latency;
        } else if (correction_on_indices_[2].empty() && correction_on_indices_[3].empty()) {
            new_latency -= unit_latency;
        }
        if (latency.exchange(new_latency, std::memory_order::relaxed) != new_latency) {
            triggerAsyncUpdate();
        }
        std::fill(to_update_correction_.begin(), to_update_correction_.end(), true);
    }

    void Controller::prepareCorrection() {
        switch (c_filter_structure_) {
            case kMatched: {
                match_calculator_.update(res_tdfs_, res_ideals_, correction_on_total_, res_update_flags_);
                break;
            }
            case kMixed: {
                mixed_calculator_.update(res_tdfs_, res_ideals_, correction_on_total_, res_update_flags_);
                break;
            }
            case kLinear: {
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
            for (const size_t &i: correction_on_indices_[lr]) {
                if (res_update_flags_[i]) {
                    res_update_flags_[i] = false;
                    to_update_correction_[lr] = true;
                }
            }
        }
        for (size_t lr = 0; lr < 4; ++lr) {
            if (to_update_correction_[lr]) {
                to_update_correction_[lr] = false;
                switch (c_filter_structure_) {
                    case kMatched: {
                        match_corrections_[lr].updateCorrection(match_calculator_.getCorrections(), correction_on_indices_[lr + 1]);
                        break;
                    }
                    case kMixed: {
                        mixed_corrections_[lr].updateCorrection(mixed_calculator_.getCorrections(), correction_on_indices_[lr + 1]);
                        break;
                    }
                    case kLinear: {
                        zero_corrections_[lr].updateCorrection(zero_calculator_.getCorrections(), correction_on_indices_[lr + 1]);
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

    void Controller::prepareSideFilters() {
        for (const size_t &i:not_off_total_) {
            if (side_emptys_[i].getUpdateParaFlag()) {
                side_filters_[i].updateParas(side_emptys_[i].getParas());
            }
        }
    }

    template<bool IsBypassed>
    void Controller::process(std::array<double *, 2> main_pointers,
                             std::array<double *, 2> side_pointers,
                             const size_t num_samples) {
        prepareBuffer();
        juce::ignoreUnused(main_pointers, side_pointers, num_samples);
    }

    template void Controller::process<true>(std::array<double *, 2>, std::array<double *, 2>, size_t);

    template void Controller::process<false>(std::array<double *, 2>, std::array<double *, 2>, size_t);

    void Controller::handleAsyncUpdate()  {
        p_ref_.setLatencySamples(latency.load(std::memory_order::relaxed));
    }
}
