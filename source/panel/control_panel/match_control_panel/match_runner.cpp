// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_runner.hpp"

namespace zlpanel {
    MatchRunner::MatchRunner(PluginProcessor &p, zlgui::UIBase &base,
                             std::array<std::atomic<float>, 251> &atomic_diffs,
                             zlgui::CompactLinearSlider &num_band_slider)
        : Thread("match_runner"), ui_base_(base),
          parameters_ref_(p.parameters_), parameters_NA_ref_(p.parameters_NA_),
          atomic_diffs_ref_(atomic_diffs),
          num_band_slider_ref_(num_band_slider) {
        parameters_NA_ref_.addParameterListener(zlstate::maximumDB::ID, this);
        parameterChanged(zlstate::maximumDB::ID, parameters_NA_ref_.getRawParameterValue(zlstate::maximumDB::ID)->load());
        std::fill(diffs_.begin(), diffs_.end(), 0.);
        ui_base_.getValueTree().addListener(this);
        addListener(&optimizer_);
    }

    MatchRunner::~MatchRunner() {
        stopThread(-1);
        removeListener(&optimizer_);
        ui_base_.getValueTree().removeListener(this);
        parameters_NA_ref_.removeParameterListener(zlstate::maximumDB::ID, this);
    }

    void MatchRunner::start() {
        startThread(Priority::low);
    }

    void MatchRunner::run() {
        const auto startIdx = static_cast<size_t>(low_cut_p_.load() * static_cast<float>(diffs_.size()));
        const auto endIdx = static_cast<size_t>(high_cut_p_.load() * static_cast<float>(diffs_.size()));
        if (mode_.load() == 0) {
            loadDiffs();
            optimizer_.setDiffs(&diffs_[0], diffs_.size());
            optimizer_.runDeterministic(startIdx, endIdx);
        } else if (mode_.load() == 1) {
            loadDiffs();
            optimizer_.setDiffs(&diffs_[0], diffs_.size());
            optimizer_.runStochastic(startIdx, endIdx);
        } else {
            loadDiffs();
            optimizer_.setDiffs(&diffs_[0], diffs_.size());
            optimizer_.runStochasticPlus({2, 4, 6}, startIdx, endIdx);
        }
        if (threadShouldExit()) {
            return;
        } {
            juce::ScopedLock lock(critical_section_);
            const auto &filters = optimizer_.getSol();
            for (size_t i = 0; i < filters_.size(); i++) {
                filters_[i].setFilterType(filters[i].getFilterType());
                filters_[i].setOrder(filters[i].getOrder());
                filters_[i].setFreq(filters[i].getFreq());
                filters_[i].setGain(filters[i].getGain());
                filters_[i].setQ(filters[i].getQ());
            }
            est_num_band_ = 16;
            const auto &mse = optimizer_.getMSE();
            const auto thresholdTemp = static_cast<double>(maximum_db_.load()) * mseRelThreshold;
            const auto threshold = thresholdTemp;
            for (size_t i = 0; i < filters_.size(); i++) {
                if (mse[i] < threshold) {
                    est_num_band_ = i + 1;
                    break;
                }
            }
            to_calculate_num_band_.store(true);
        }
        triggerAsyncUpdate();
    }

    void MatchRunner::handleAsyncUpdate() {
        juce::ScopedLock lock(critical_section_);
        size_t currentNumBand = num_band_.load();
        if (to_calculate_num_band_.exchange(false)) {
            currentNumBand = est_num_band_;
            size_t currentMaxBand = filters_.size();
            for (size_t i = 0; i < filters_.size(); i++) {
                if (std::abs(filters_[i].getGain()) < 1e-6) {
                    currentMaxBand = i;
                    break;
                }
            }
            num_band_slider_ref_.getSlider().setRange(1.0, static_cast<double>(currentMaxBand), 1.0);
            num_band_slider_ref_.getSlider().setValue(static_cast<double>(currentNumBand), juce::dontSendNotification);
            num_band_slider_ref_.getSlider().setDoubleClickReturnValue(true, static_cast<double>(currentNumBand));
            num_band_slider_ref_.updateDisplayValue();
            num_band_.store(currentNumBand);
            ui_base_.setProperty(zlgui::SettingIdx::kMatchFitRunning, false);
        }
        for (size_t i = 0; i < currentNumBand; i++) {
            const auto &filter = filters_[i];
            savePara(zlp::appendSuffix(zlp::bypass::ID, i), 0.f);
            savePara(zlp::appendSuffix(zlp::dynamicON::ID, i), 0.f);
            savePara(zlp::appendSuffix(zlp::fType::ID, i),
                     zlp::fType::convertTo01(filter.getFilterType()));
            savePara(zlp::appendSuffix(zlp::slope::ID, i),
                     zlp::slope::convertTo01(zlp::slope::convertToIdx(filter.getOrder())));
            savePara(zlp::appendSuffix(zlp::freq::ID, i),
                     zlp::freq::convertTo01(static_cast<float>(filter.getFreq())));
            savePara(zlp::appendSuffix(zlp::gain::ID, i),
                     zlp::gain::convertTo01(static_cast<float>(filter.getGain())));
            savePara(zlp::appendSuffix(zlp::Q::ID, i),
                     zlp::Q::convertTo01(static_cast<float>(filter.getQ())));
        }
        for (size_t i = currentNumBand; i < filters_.size(); i++) {
            const auto para = parameters_NA_ref_.getParameter(zlstate::appendSuffix(zlstate::active::ID, i));
            para->beginChangeGesture();
            para->setValueNotifyingHost(0.f);
            para->endChangeGesture();
        }
    }

    void MatchRunner::valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                               const juce::Identifier &property) {
        juce::ignoreUnused(tree_whose_property_has_changed);
        if (property == zlgui::kMatchIdentifiers[static_cast<size_t>(zlgui::SettingIdx::kMatchLowCut)]) {
            low_cut_p_.store(std::clamp(
                static_cast<float>(ui_base_.getProperty(zlgui::SettingIdx::kMatchLowCut)), 0.f, 1.f));
        } else if (property == zlgui::kMatchIdentifiers[static_cast<size_t>(zlgui::SettingIdx::kMatchHighCut)]) {
            high_cut_p_.store(std::clamp(
                static_cast<float>(ui_base_.getProperty(zlgui::SettingIdx::kMatchHighCut)), 0.f, 1.f));
        }
    }

    void MatchRunner::loadDiffs() {
        for (size_t i = 0; i < diffs_.size(); i++) {
            diffs_[i] = static_cast<double>(atomic_diffs_ref_[i].load());
        }
    }

    void MatchRunner::parameterChanged(const juce::String &parameter_id, const float new_value) {
        juce::ignoreUnused(parameter_id);
        maximum_db_.store(zlstate::maximumDB::dBs[static_cast<size_t>(new_value)]);
    }
} // zlpanel
