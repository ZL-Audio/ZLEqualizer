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
                                            juce::AudioProcessorValueTreeState &parametersNA,
                                            Controller<FloatType> &controller)
        : processor_ref(processor), parameters_ref(parameters), parameters_NA_ref(parametersNA),
          controller_ref(controller), filtersRef(controller.getFilters()) {
        addListeners();
        initDefaultValues();
        for (size_t i = 0; i < bandNUM; ++i) {
            const auto suffix = zlp::appendSuffix("", i);
            sideFreqUpdater[i] = std::make_unique<zldsp::chore::ParaUpdater>(parameters, sideFreq::ID + suffix);
            sideQUpdater[i] = std::make_unique<zldsp::chore::ParaUpdater>(parameters, sideQ::ID + suffix);
            thresholdUpdater[i] = std::make_unique<zldsp::chore::ParaUpdater>(parameters, threshold::ID + suffix);
            kneeUpdater[i] = std::make_unique<zldsp::chore::ParaUpdater>(parameters, kneeW::ID + suffix);
        }
    }

    template<typename FloatType>
    FiltersAttach<FloatType>::~FiltersAttach() {
        for (size_t i = 0; i < bandNUM; ++i) {
            const auto suffix = appendSuffix("", i);
            for (auto &ID: IDs) {
                parameters_ref.removeParameterListener(ID + suffix, this);
            }
        }
        parameters_NA_ref.removeParameterListener(zlstate::maximumDB::ID, this);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::addListeners() {
        for (size_t i = 0; i < bandNUM; ++i) {
            const auto suffix = appendSuffix("", i);
            for (auto &ID: IDs) {
                parameters_ref.addParameterListener(ID + suffix, this);
            }
            parameters_NA_ref.addParameterListener(zlstate::active::ID + suffix, this);
        }
        parameters_NA_ref.addParameterListener(zlstate::maximumDB::ID, this);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::turnOnDynamic(const size_t idx) {
        updateTargetFGQ(idx);
        updateSideFQ(idx);
        const auto paraDynBypass = parameters_ref.getParameter(zlp::appendSuffix(dynamicBypass::ID, idx));
        updateParaNotifyHost(paraDynBypass, 0.f);
        const auto paraDynLearn = parameters_ref.getParameter(zlp::appendSuffix(dynamicLearn::ID, idx));
        updateParaNotifyHost(paraDynLearn, 1.f);
        const auto paraThreshold = parameters_ref.getParameter(zlp::appendSuffix(threshold::ID, idx));
        updateParaNotifyHost(paraThreshold, .5f);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::turnOffDynamic(const size_t idx) {
        controller_ref.setDynamicON(false, idx);
        const auto paraDynBypass = parameters_ref.getParameter(zlp::appendSuffix(dynamicBypass::ID, idx));
        updateParaNotifyHost(paraDynBypass, 1.f);
        const auto paraDynLearn = parameters_ref.getParameter(zlp::appendSuffix(dynamicLearn::ID, idx));
        updateParaNotifyHost(paraDynLearn, 0.f);
        const auto paraDynRelative = parameters_ref.getParameter(zlp::appendSuffix(dynamicRelative::ID, idx));
        updateParaNotifyHost(paraDynRelative, 0.f);
        const auto paraSideSwap = parameters_ref.getParameter(zlp::appendSuffix(sideSwap::ID, idx));
        updateParaNotifyHost(paraSideSwap, 0.f);
        const auto paraSideSolo = parameters_ref.getParameter(zlp::appendSuffix(sideSolo::ID, idx));
        updateParaNotifyHost(paraSideSolo, 0.f);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::turnOnDynamicAuto(const size_t idx) {
        const auto paraThreshold = parameters_ref.getParameter(zlp::appendSuffix(threshold::ID, idx));
        updateParaNotifyHost(paraThreshold, .5f);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::updateTargetFGQ(const size_t idx) {
        auto tGain = static_cast<float>(controller_ref.getBaseFilter(idx).getGain());
        switch (controller_ref.getBaseFilter(idx).getFilterType()) {
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kBandShelf: {
                const auto maxDB = maximumDB.load();
                if (tGain < -maxDB * .5f) {
                    tGain = juce::jlimit(-maxDB, maxDB, tGain -= maxDB * .125f);
                } else if (tGain < 0) {
                    tGain += maxDB * .125f;
                } else if (tGain < maxDB * .5f) {
                    tGain -= maxDB * .125f;
                } else {
                    tGain = juce::jlimit(-maxDB, maxDB, tGain += maxDB * .125f);
                }
                break;
            }
            case zldsp::filter::FilterType::kLowShelf:
            case zldsp::filter::FilterType::kHighShelf:
            case zldsp::filter::FilterType::kTiltShelf: {
                if (tGain < 0) {
                    tGain += maximumDB.load() * .25f;
                } else {
                    tGain -= maximumDB.load() * .25f;
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
        controller_ref.getTargetFilter(idx).setFreq(controller_ref.getBaseFilter(idx).getFreq());
        controller_ref.getTargetFilter(idx).setFilterType(controller_ref.getBaseFilter(idx).getFilterType());
        controller_ref.getTargetFilter(idx).setOrder(controller_ref.getBaseFilter(idx).getOrder());
        const auto paraGain = parameters_ref.getParameter(zlp::appendSuffix(targetGain::ID, idx));
        updateParaNotifyHost(paraGain, targetGain::convertTo01(tGain));
        const auto paraQ = parameters_ref.getParameter(zlp::appendSuffix(targetQ::ID, idx));
        updateParaNotifyHost(paraQ, targetQ::convertTo01(static_cast<float>(controller_ref.getBaseFilter(idx).getQ())));
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::updateSideFQ(const size_t idx) {
        const auto &f{controller_ref.getBaseFilter(idx)};
        auto [soloFreq, soloQ] = controller_ref.getSoloFilterParas(
            f.getFilterType(), f.getFreq(), f.getQ());
        const auto soloFreq01 = sideFreq::convertTo01(static_cast<float>(soloFreq));
        const auto soloQ01 = sideQ::convertTo01(static_cast<float>(soloQ));
        sideFreqUpdater[idx]->update(soloFreq01);
        sideQUpdater[idx]->update(soloQ01);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlstate::maximumDB::ID) {
            maximumDB.store(zlstate::maximumDB::dBs[static_cast<size_t>(newValue)]);
            return;
        }
        const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        auto value = static_cast<FloatType>(newValue);
        if (parameterID.startsWith(zlstate::active::ID)) {
            controller_ref.setIsActive(idx, newValue > .5f);
        } else if (parameterID.startsWith(bypass::ID)) {
            controller_ref.setBypass(idx, newValue > .5f);
        } else if (parameterID.startsWith(fType::ID)) {
            const auto fType = static_cast<zldsp::filter::FilterType>(value);
            controller_ref.getBaseFilter(idx).setFilterType(fType);
            filtersRef[idx].getMainFilter().setFilterType(fType);
            controller_ref.getTargetFilter(idx).setFilterType(fType);
            controller_ref.getMainIdealFilter(idx).setFilterType(fType);
            controller_ref.getMainIIRFilter(idx).setFilterType(fType);
            if (sDynLink[idx].load()) {
                updateSideFQ(idx);
            }
            controller_ref.updateSgc(idx);
        } else if (parameterID.startsWith(slope::ID)) {
            const auto newOrder = slope::orderArray[static_cast<size_t>(value)];
            controller_ref.getBaseFilter(idx).setOrder(newOrder);
            filtersRef[idx].getMainFilter().setOrder(newOrder);
            controller_ref.getTargetFilter(idx).setOrder(newOrder);
            controller_ref.getMainIdealFilter(idx).setOrder(newOrder);
            controller_ref.getMainIIRFilter(idx).setOrder(newOrder);
        } else if (parameterID.startsWith(freq::ID)) {
            controller_ref.getBaseFilter(idx).setFreq(value);
            filtersRef[idx].getMainFilter().setFreq(value);
            controller_ref.getTargetFilter(idx).setFreq(value);
            controller_ref.getMainIdealFilter(idx).setFreq(value);
            controller_ref.getMainIIRFilter(idx).setFreq(value);
            if (sDynLink[idx].load()) {
                updateSideFQ(idx);
            }
            controller_ref.updateSgc(idx);
        } else if (parameterID.startsWith(gain::ID)) {
            value *= static_cast<FloatType>(scale::formatV(parameters_ref.getRawParameterValue(scale::ID)->load()));
            value = gain::range.snapToLegalValue(static_cast<float>(value));
            controller_ref.getBaseFilter(idx).setGain(value);
            filtersRef[idx].getMainFilter().setGain(value);
            controller_ref.getMainIdealFilter(idx).setGain(value);
            controller_ref.getMainIIRFilter(idx).setGain(value);
            controller_ref.updateSgc(idx);
        } else if (parameterID.startsWith(Q::ID)) {
            controller_ref.getBaseFilter(idx).setQ(value);
            filtersRef[idx].getMainFilter().setQ(value);
            controller_ref.getMainIdealFilter(idx).setQ(value);
            controller_ref.getMainIIRFilter(idx).setQ(value);
            if (sDynLink[idx].load()) {
                updateSideFQ(idx);
            }
            controller_ref.updateSgc(idx);
        } else if (parameterID.startsWith(lrType::ID)) {
            controller_ref.setFilterLRs(static_cast<lrType::lrTypes>(value), idx);
        } else if (parameterID.startsWith(dynamicON::ID)) {
            controller_ref.setDynamicON(newValue > .5f, idx);
        } else if (parameterID.startsWith(dynamicLearn::ID)) {
            const auto f = newValue > .5f;
            if (!f && controller_ref.getLearningHistON(idx)) {
                controller_ref.setLearningHistON(idx, false);
                const auto &hist = controller_ref.getLearningHist(idx);
                const auto thresholdV = static_cast<float>(
                    -hist.getPercentile(FloatType(0.5)) + controller_ref.getThreshold(idx) + FloatType(40));
                const auto kneeV = static_cast<float>(
                                       hist.getPercentile(FloatType(0.95)) - hist.getPercentile(FloatType(0.05))
                                   ) / 120.f;
                thresholdUpdater[idx]->update(threshold::convertTo01(threshold::range.snapToLegalValue(thresholdV)));
                kneeUpdater[idx]->update(kneeW::convertTo01(kneeW::range.snapToLegalValue(kneeV)));
            } else {
                controller_ref.setLearningHistON(idx, f);
            }
        } else if (parameterID.startsWith(dynamicBypass::ID)) {
            filtersRef[idx].setDynamicBypass(newValue > .5f);
        } else if (parameterID.startsWith(dynamicRelative::ID)) {
            controller_ref.setRelative(idx, newValue > .5f);
        } else if (parameterID.startsWith(sideSwap::ID)) {
            controller_ref.setSideSwap(idx, newValue > .5f);
        } else if (parameterID.startsWith(targetGain::ID)) {
            value *= static_cast<FloatType>(scale::formatV(parameters_ref.getRawParameterValue(scale::ID)->load()));
            value = targetGain::range.snapToLegalValue(static_cast<float>(value));
            controller_ref.getTargetFilter(idx).setGain(value);
        } else if (parameterID.startsWith(targetQ::ID)) {
            controller_ref.getTargetFilter(idx).setQ(value);
        } else if (parameterID.startsWith(threshold::ID)) {
            controller_ref.setThreshold(idx, value);
            filtersRef[idx].getComputer().setThreshold(value);
        } else if (parameterID.startsWith(kneeW::ID)) {
            filtersRef[idx].getComputer().setKneeW(kneeW::formatV(value));
        } else if (parameterID.startsWith(sideFreq::ID)) {
            filtersRef[idx].getSideFilter().setFreq(value);
        } else if (parameterID.startsWith(attack::ID)) {
            filtersRef[idx].getFollower().setAttack(value);
        } else if (parameterID.startsWith(release::ID)) {
            filtersRef[idx].getFollower().setRelease(value);
        } else if (parameterID.startsWith(sideQ::ID)) {
            filtersRef[idx].getSideFilter().setQ(value);
        } else if (parameterID.startsWith(singleDynLink::ID)) {
            sDynLink[idx].store(newValue > .5f);
            if (sDynLink[idx].load()) {
                updateSideFQ(idx);
            }
        }
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::initDefaultValues() {
        for (int i = 0; i < bandNUM; ++i) {
            auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            for (size_t j = 0; j < defaultVs.size(); ++j) {
                parameterChanged(IDs[j] + suffix, defaultVs[j]);
            }
        }
    }

    template
    class FiltersAttach<float>;

    template
    class FiltersAttach<double>;
}
