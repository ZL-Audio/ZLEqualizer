// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "filters_attach.hpp"

namespace zlp {
    template<typename FloatType>
    FiltersAttach<FloatType>::FiltersAttach(juce::AudioProcessor &processor,
                                            juce::AudioProcessorValueTreeState &parameters,
                                            juce::AudioProcessorValueTreeState &parameters_NA,
                                            Controller<FloatType> &controller)
        : processor_ref_(processor), parameters_ref_(parameters), parameters_NA_ref_(parameters_NA),
          controller_ref_(controller), filters_ref_(controller.getFilters()) {
        addListeners();
        initDefaultValues();
        for (size_t i = 0; i < kBandNUM; ++i) {
            const auto suffix = zlp::appendSuffix("", i);
            side_freq_updater_[i] = std::make_unique<zldsp::chore::ParaUpdater>(parameters, sideFreq::ID + suffix);
            side_q_updater_[i] = std::make_unique<zldsp::chore::ParaUpdater>(parameters, sideQ::ID + suffix);
            threshold_updater_[i] = std::make_unique<zldsp::chore::ParaUpdater>(parameters, threshold::ID + suffix);
            knee_updater_[i] = std::make_unique<zldsp::chore::ParaUpdater>(parameters, kneeW::ID + suffix);
        }
    }

    template<typename FloatType>
    FiltersAttach<FloatType>::~FiltersAttach() {
        for (size_t i = 0; i < kBandNUM; ++i) {
            const auto suffix = appendSuffix("", i);
            for (auto &ID: kIDs) {
                parameters_ref_.removeParameterListener(ID + suffix, this);
            }
        }
        parameters_NA_ref_.removeParameterListener(zlstate::maximumDB::ID, this);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::addListeners() {
        for (size_t i = 0; i < kBandNUM; ++i) {
            const auto suffix = appendSuffix("", i);
            for (auto &ID: kIDs) {
                parameters_ref_.addParameterListener(ID + suffix, this);
            }
            parameters_NA_ref_.addParameterListener(zlstate::active::ID + suffix, this);
        }
        parameters_NA_ref_.addParameterListener(zlstate::maximumDB::ID, this);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::turnOnDynamic(const size_t idx) {
        updateTargetFGQ(idx);
        updateSideFQ(idx);
        const auto para_dyn_bypass = parameters_ref_.getParameter(zlp::appendSuffix(dynamicBypass::ID, idx));
        updateParaNotifyHost(para_dyn_bypass, 0.f);
        const auto para_dyn_learn = parameters_ref_.getParameter(zlp::appendSuffix(dynamicLearn::ID, idx));
        updateParaNotifyHost(para_dyn_learn, 1.f);
        const auto para_threshold = parameters_ref_.getParameter(zlp::appendSuffix(threshold::ID, idx));
        updateParaNotifyHost(para_threshold, .5f);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::turnOffDynamic(const size_t idx) {
        controller_ref_.setDynamicON(false, idx);
        const auto para_dyn_bypass = parameters_ref_.getParameter(zlp::appendSuffix(dynamicBypass::ID, idx));
        updateParaNotifyHost(para_dyn_bypass, 1.f);
        const auto para_dyn_learn = parameters_ref_.getParameter(zlp::appendSuffix(dynamicLearn::ID, idx));
        updateParaNotifyHost(para_dyn_learn, 0.f);
        const auto para_dyn_relative = parameters_ref_.getParameter(zlp::appendSuffix(dynamicRelative::ID, idx));
        updateParaNotifyHost(para_dyn_relative, 0.f);
        const auto para_side_swap = parameters_ref_.getParameter(zlp::appendSuffix(sideSwap::ID, idx));
        updateParaNotifyHost(para_side_swap, 0.f);
        const auto para_side_solo = parameters_ref_.getParameter(zlp::appendSuffix(sideSolo::ID, idx));
        updateParaNotifyHost(para_side_solo, 0.f);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::turnOnDynamicAuto(const size_t idx) {
        const auto para_threshold = parameters_ref_.getParameter(zlp::appendSuffix(threshold::ID, idx));
        updateParaNotifyHost(para_threshold, .5f);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::updateTargetFGQ(const size_t idx) {
        auto t_gain = static_cast<float>(controller_ref_.getBaseFilter(idx).getGain());
        switch (controller_ref_.getBaseFilter(idx).getFilterType()) {
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kBandShelf: {
                const auto maxDB = maximum_db_.load();
                if (t_gain < -maxDB * .5f) {
                    t_gain = juce::jlimit(-maxDB, maxDB, t_gain -= maxDB * .125f);
                } else if (t_gain < 0) {
                    t_gain += maxDB * .125f;
                } else if (t_gain < maxDB * .5f) {
                    t_gain -= maxDB * .125f;
                } else {
                    t_gain = juce::jlimit(-maxDB, maxDB, t_gain += maxDB * .125f);
                }
                break;
            }
            case zldsp::filter::FilterType::kLowShelf:
            case zldsp::filter::FilterType::kHighShelf:
            case zldsp::filter::FilterType::kTiltShelf: {
                if (t_gain < 0) {
                    t_gain += maximum_db_.load() * .25f;
                } else {
                    t_gain -= maximum_db_.load() * .25f;
                }
                break;
            }
            case zldsp::filter::FilterType::kLowPass:
            case zldsp::filter::FilterType::kHighPass:
            case zldsp::filter::FilterType::kNotch:
            case zldsp::filter::FilterType::kBandPass:
            default: {
                break;
            }
        }
        controller_ref_.getTargetFilter(idx).setFreq(controller_ref_.getBaseFilter(idx).getFreq());
        controller_ref_.getTargetFilter(idx).setFilterType(controller_ref_.getBaseFilter(idx).getFilterType());
        controller_ref_.getTargetFilter(idx).setOrder(controller_ref_.getBaseFilter(idx).getOrder());
        const auto para_gain = parameters_ref_.getParameter(zlp::appendSuffix(targetGain::ID, idx));
        updateParaNotifyHost(para_gain, targetGain::convertTo01(t_gain));
        const auto para_q = parameters_ref_.getParameter(zlp::appendSuffix(targetQ::ID, idx));
        updateParaNotifyHost(para_q, targetQ::convertTo01(static_cast<float>(controller_ref_.getBaseFilter(idx).getQ())));
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::updateSideFQ(const size_t idx) {
        const auto &f{controller_ref_.getBaseFilter(idx)};
        auto [solo_freq, solo_q] = controller_ref_.getSoloFilterParas(
            f.getFilterType(), f.getFreq(), f.getQ());
        const auto solo_freq01 = sideFreq::convertTo01(static_cast<float>(solo_freq));
        const auto solo_q01 = sideQ::convertTo01(static_cast<float>(solo_q));
        side_freq_updater_[idx]->update(solo_freq01);
        side_q_updater_[idx]->update(solo_q01);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::parameterChanged(const juce::String &parameter_id, float new_value) {
        if (parameter_id == zlstate::maximumDB::ID) {
            maximum_db_.store(zlstate::maximumDB::dBs[static_cast<size_t>(new_value)]);
            return;
        }
        const auto idx = static_cast<size_t>(parameter_id.getTrailingIntValue());
        auto value = static_cast<FloatType>(new_value);
        if (parameter_id.startsWith(zlstate::active::ID)) {
            controller_ref_.setIsActive(idx, new_value > .5f);
        } else if (parameter_id.startsWith(bypass::ID)) {
            controller_ref_.setBypass(idx, new_value > .5f);
        } else if (parameter_id.startsWith(fType::ID)) {
            const auto fType = static_cast<zldsp::filter::FilterType>(value);
            controller_ref_.getBaseFilter(idx).setFilterType(fType);
            filters_ref_[idx].getMainFilter().setFilterType(fType);
            controller_ref_.getTargetFilter(idx).setFilterType(fType);
            controller_ref_.getMainIdealFilter(idx).setFilterType(fType);
            controller_ref_.getMainIIRFilter(idx).setFilterType(fType);
            if (s_dyn_link_[idx].load()) {
                updateSideFQ(idx);
            }
            controller_ref_.updateSgc(idx);
        } else if (parameter_id.startsWith(slope::ID)) {
            const auto newOrder = slope::orderArray[static_cast<size_t>(value)];
            controller_ref_.getBaseFilter(idx).setOrder(newOrder);
            filters_ref_[idx].getMainFilter().setOrder(newOrder);
            controller_ref_.getTargetFilter(idx).setOrder(newOrder);
            controller_ref_.getMainIdealFilter(idx).setOrder(newOrder);
            controller_ref_.getMainIIRFilter(idx).setOrder(newOrder);
        } else if (parameter_id.startsWith(freq::ID)) {
            controller_ref_.getBaseFilter(idx).setFreq(value);
            filters_ref_[idx].getMainFilter().setFreq(value);
            controller_ref_.getTargetFilter(idx).setFreq(value);
            controller_ref_.getMainIdealFilter(idx).setFreq(value);
            controller_ref_.getMainIIRFilter(idx).setFreq(value);
            if (s_dyn_link_[idx].load()) {
                updateSideFQ(idx);
            }
            controller_ref_.updateSgc(idx);
        } else if (parameter_id.startsWith(gain::ID)) {
            value *= static_cast<FloatType>(scale::formatV(parameters_ref_.getRawParameterValue(scale::ID)->load()));
            value = gain::range.snapToLegalValue(static_cast<float>(value));
            controller_ref_.getBaseFilter(idx).setGain(value);
            filters_ref_[idx].getMainFilter().setGain(value);
            controller_ref_.getMainIdealFilter(idx).setGain(value);
            controller_ref_.getMainIIRFilter(idx).setGain(value);
            controller_ref_.updateSgc(idx);
        } else if (parameter_id.startsWith(Q::ID)) {
            controller_ref_.getBaseFilter(idx).setQ(value);
            filters_ref_[idx].getMainFilter().setQ(value);
            controller_ref_.getMainIdealFilter(idx).setQ(value);
            controller_ref_.getMainIIRFilter(idx).setQ(value);
            if (s_dyn_link_[idx].load()) {
                updateSideFQ(idx);
            }
            controller_ref_.updateSgc(idx);
        } else if (parameter_id.startsWith(lrType::ID)) {
            controller_ref_.setFilterLRs(static_cast<lrType::lrTypes>(value), idx);
        } else if (parameter_id.startsWith(dynamicON::ID)) {
            controller_ref_.setDynamicON(new_value > .5f, idx);
        } else if (parameter_id.startsWith(dynamicLearn::ID)) {
            const auto f = new_value > .5f;
            if (!f && controller_ref_.getLearningHistON(idx)) {
                controller_ref_.setLearningHistON(idx, false);
                const auto &hist = controller_ref_.getLearningHist(idx);
                const auto thresholdV = static_cast<float>(
                    -hist.getPercentile(FloatType(0.5)) + controller_ref_.getThreshold(idx) + FloatType(40));
                const auto kneeV = static_cast<float>(
                                       hist.getPercentile(FloatType(0.95)) - hist.getPercentile(FloatType(0.05))
                                   ) / 120.f;
                threshold_updater_[idx]->update(threshold::convertTo01(threshold::range.snapToLegalValue(thresholdV)));
                knee_updater_[idx]->update(kneeW::convertTo01(kneeW::range.snapToLegalValue(kneeV)));
            } else {
                controller_ref_.setLearningHistON(idx, f);
            }
        } else if (parameter_id.startsWith(dynamicBypass::ID)) {
            filters_ref_[idx].setDynamicBypass(new_value > .5f);
        } else if (parameter_id.startsWith(dynamicRelative::ID)) {
            controller_ref_.setRelative(idx, new_value > .5f);
        } else if (parameter_id.startsWith(sideSwap::ID)) {
            controller_ref_.setSideSwap(idx, new_value > .5f);
        } else if (parameter_id.startsWith(targetGain::ID)) {
            value *= static_cast<FloatType>(scale::formatV(parameters_ref_.getRawParameterValue(scale::ID)->load()));
            value = targetGain::range.snapToLegalValue(static_cast<float>(value));
            controller_ref_.getTargetFilter(idx).setGain(value);
        } else if (parameter_id.startsWith(targetQ::ID)) {
            controller_ref_.getTargetFilter(idx).setQ(value);
        } else if (parameter_id.startsWith(threshold::ID)) {
            controller_ref_.setThreshold(idx, value);
            filters_ref_[idx].getComputer().setThreshold(value);
        } else if (parameter_id.startsWith(kneeW::ID)) {
            filters_ref_[idx].getComputer().setKneeW(kneeW::formatV(value));
        } else if (parameter_id.startsWith(sideFreq::ID)) {
            filters_ref_[idx].getSideFilter().setFreq(value);
        } else if (parameter_id.startsWith(attack::ID)) {
            filters_ref_[idx].getFollower().setAttack(value);
        } else if (parameter_id.startsWith(release::ID)) {
            filters_ref_[idx].getFollower().setRelease(value);
        } else if (parameter_id.startsWith(sideQ::ID)) {
            filters_ref_[idx].getSideFilter().setQ(value);
        } else if (parameter_id.startsWith(singleDynLink::ID)) {
            s_dyn_link_[idx].store(new_value > .5f);
            if (s_dyn_link_[idx].load()) {
                updateSideFQ(idx);
            }
        }
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::initDefaultValues() {
        for (int i = 0; i < kBandNUM; ++i) {
            auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            for (size_t j = 0; j < kDefaultVs.size(); ++j) {
                parameterChanged(kIDs[j] + suffix, kDefaultVs[j]);
            }
        }
    }

    template
    class FiltersAttach<float>;

    template
    class FiltersAttach<double>;
}
