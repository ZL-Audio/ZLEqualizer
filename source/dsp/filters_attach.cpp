// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "filters_attach.hpp"

namespace zlDSP {
    template<typename FloatType>
    FiltersAttach<FloatType>::FiltersAttach(juce::AudioProcessor &processor,
                                            juce::AudioProcessorValueTreeState &parameters,
                                            juce::AudioProcessorValueTreeState &parametersNA,
                                            Controller<FloatType> &controller)
        : processorRef(processor), parameterRef(parameters), parameterNARef(parametersNA),
          controllerRef(controller), filtersRef(controller.getFilters()) {
        addListeners();
        initDefaultValues();
        for (size_t i = 0; i < bandNUM; ++i) {
            const auto suffix = zlDSP::appendSuffix("", i);
            sideFreqUpdater[i] = std::make_unique<zlChore::ParaUpdater>(parameters, sideFreq::ID + suffix);
            sideQUpdater[i] = std::make_unique<zlChore::ParaUpdater>(parameters, sideQ::ID + suffix);
            thresholdUpdater[i] = std::make_unique<zlChore::ParaUpdater>(parameters, threshold::ID + suffix);
            kneeUpdater[i] = std::make_unique<zlChore::ParaUpdater>(parameters, kneeW::ID + suffix);
        }
    }

    template<typename FloatType>
    FiltersAttach<FloatType>::~FiltersAttach() {
        for (size_t i = 0; i < bandNUM; ++i) {
            const auto suffix = appendSuffix("", i);
            for (auto &ID: IDs) {
                parameterRef.removeParameterListener(ID + suffix, this);
            }
        }
        parameterNARef.removeParameterListener(zlState::maximumDB::ID, this);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::addListeners() {
        for (size_t i = 0; i < bandNUM; ++i) {
            const auto suffix = appendSuffix("", i);
            for (auto &ID: IDs) {
                parameterRef.addParameterListener(ID + suffix, this);
            }
            parameterNARef.addParameterListener(zlState::active::ID + suffix, this);
        }
        parameterNARef.addParameterListener(zlState::maximumDB::ID, this);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::turnOnDynamic(const size_t idx) {
        updateTargetFGQ(idx);
        updateSideFQ(idx);
        const auto paraDynBypass = parameterRef.getParameter(zlDSP::appendSuffix(dynamicBypass::ID, idx));
        updateParaNotifyHost(paraDynBypass, 0.f);
        const auto paraDynLearn = parameterRef.getParameter(zlDSP::appendSuffix(dynamicLearn::ID, idx));
        updateParaNotifyHost(paraDynLearn, 1.f);
        const auto paraThreshold = parameterRef.getParameter(zlDSP::appendSuffix(threshold::ID, idx));
        updateParaNotifyHost(paraThreshold, .5f);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::turnOffDynamic(const size_t idx) {
        controllerRef.setDynamicON(false, idx);
        const auto paraDynBypass = parameterRef.getParameter(zlDSP::appendSuffix(dynamicBypass::ID, idx));
        updateParaNotifyHost(paraDynBypass, 1.f);
        const auto paraDynLearn = parameterRef.getParameter(zlDSP::appendSuffix(dynamicLearn::ID, idx));
        updateParaNotifyHost(paraDynLearn, 0.f);
        const auto paraDynRelative = parameterRef.getParameter(zlDSP::appendSuffix(dynamicRelative::ID, idx));
        updateParaNotifyHost(paraDynRelative, 0.f);
        const auto paraSideSolo = parameterRef.getParameter(zlDSP::appendSuffix(sideSolo::ID, idx));
        updateParaNotifyHost(paraSideSolo, 0.f);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::turnOnDynamicAuto(const size_t idx) {
        const auto paraThreshold = parameterRef.getParameter(zlDSP::appendSuffix(threshold::ID, idx));
        updateParaNotifyHost(paraThreshold, .5f);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::updateTargetFGQ(const size_t idx) {
        auto tGain = static_cast<float>(controllerRef.getBaseFilter(idx).getGain());
        switch (controllerRef.getBaseFilter(idx).getFilterType()) {
            case zlFilter::FilterType::peak:
            case zlFilter::FilterType::bandShelf: {
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
            case zlFilter::FilterType::lowShelf:
            case zlFilter::FilterType::highShelf:
            case zlFilter::FilterType::tiltShelf: {
                if (tGain < 0) {
                    tGain += maximumDB.load() * .25f;
                } else {
                    tGain -= maximumDB.load() * .25f;
                }
                break;
            }
            case zlFilter::FilterType::lowPass:
            case zlFilter::FilterType::highPass:
            case zlFilter::FilterType::notch:
            case zlFilter::FilterType::bandPass:
            default: {
                break;
            }
        }
        controllerRef.getTargetFilter(idx).setFreq(controllerRef.getBaseFilter(idx).getFreq());
        controllerRef.getTargetFilter(idx).setFilterType(controllerRef.getBaseFilter(idx).getFilterType());
        controllerRef.getTargetFilter(idx).setOrder(controllerRef.getBaseFilter(idx).getOrder());
        const auto paraGain = parameterRef.getParameter(zlDSP::appendSuffix(targetGain::ID, idx));
        updateParaNotifyHost(paraGain, targetGain::convertTo01(tGain));
        const auto paraQ = parameterRef.getParameter(zlDSP::appendSuffix(targetQ::ID, idx));
        updateParaNotifyHost(paraQ, targetQ::convertTo01(static_cast<float>(controllerRef.getBaseFilter(idx).getQ())));
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::updateSideFQ(const size_t idx) {
        const auto &f{controllerRef.getBaseFilter(idx)};
        auto [soloFreq, soloQ] = controllerRef.getSoloFilterParas(
            f.getFilterType(), f.getFreq(), f.getQ());
        const auto soloFreq01 = sideFreq::convertTo01(static_cast<float>(soloFreq));
        const auto soloQ01 = sideQ::convertTo01(static_cast<float>(soloQ));
        sideFreqUpdater[idx]->update(soloFreq01);
        sideQUpdater[idx]->update(soloQ01);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlState::maximumDB::ID) {
            maximumDB.store(zlState::maximumDB::dBs[static_cast<size_t>(newValue)]);
            return;
        }
        const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        auto value = static_cast<FloatType>(newValue);
        if (parameterID.startsWith(zlState::active::ID)) {
            controllerRef.setIsActive(idx, newValue > .5f);
        } else if (parameterID.startsWith(bypass::ID)) {
            controllerRef.setBypass(idx, newValue > .5f);
        } else if (parameterID.startsWith(fType::ID)) {
            const auto fType = static_cast<zlFilter::FilterType>(value);
            controllerRef.getBaseFilter(idx).setFilterType(fType);
            filtersRef[idx].getMainFilter().setFilterType(fType);
            controllerRef.getTargetFilter(idx).setFilterType(fType);
            controllerRef.getMainIdealFilter(idx).setFilterType(fType);
            controllerRef.getMainIIRFilter(idx).setFilterType(fType);
            controllerRef.updateSgc(idx);
        } else if (parameterID.startsWith(slope::ID)) {
            const auto newOrder = slope::orderArray[static_cast<size_t>(value)];
            controllerRef.getBaseFilter(idx).setOrder(newOrder);
            filtersRef[idx].getMainFilter().setOrder(newOrder);
            controllerRef.getTargetFilter(idx).setOrder(newOrder);
            controllerRef.getMainIdealFilter(idx).setOrder(newOrder);
            controllerRef.getMainIIRFilter(idx).setOrder(newOrder);
        } else if (parameterID.startsWith(freq::ID)) {
            controllerRef.getBaseFilter(idx).setFreq(value);
            filtersRef[idx].getMainFilter().setFreq(value);
            controllerRef.getTargetFilter(idx).setFreq(value);
            controllerRef.getMainIdealFilter(idx).setFreq(value);
            controllerRef.getMainIIRFilter(idx).setFreq(value);
            if (sDynLink[idx].load()) {
                updateSideFQ(idx);
            }
            controllerRef.updateSgc(idx);
        } else if (parameterID.startsWith(gain::ID)) {
            value *= static_cast<FloatType>(scale::formatV(parameterRef.getRawParameterValue(scale::ID)->load()));
            value = gain::range.snapToLegalValue(static_cast<float>(value));
            controllerRef.getBaseFilter(idx).setGain(value);
            filtersRef[idx].getMainFilter().setGain(value);
            controllerRef.getMainIdealFilter(idx).setGain(value);
            controllerRef.getMainIIRFilter(idx).setGain(value);
            controllerRef.updateSgc(idx);
        } else if (parameterID.startsWith(Q::ID)) {
            controllerRef.getBaseFilter(idx).setQ(value);
            filtersRef[idx].getMainFilter().setQ(value);
            filtersRef[idx].updateIsCurrentDynamicChangeQ();
            controllerRef.getMainIdealFilter(idx).setQ(value);
            controllerRef.getMainIIRFilter(idx).setQ(value);
            if (sDynLink[idx].load()) {
                updateSideFQ(idx);
            }
            controllerRef.updateSgc(idx);
        } else if (parameterID.startsWith(lrType::ID)) {
            controllerRef.setFilterLRs(static_cast<lrType::lrTypes>(value), idx);
        } else if (parameterID.startsWith(dynamicON::ID)) {
            controllerRef.setDynamicON(newValue > .5f, idx);
        } else if (parameterID.startsWith(dynamicLearn::ID)) {
            const auto f = newValue > .5f;
            if (!f && controllerRef.getLearningHistON(idx)) {
                controllerRef.setLearningHist(idx, false);
                const auto &hist = controllerRef.getLearningHist(idx);
                const auto thresholdV = static_cast<float>(
                    -hist.getPercentile(FloatType(0.5)) + controllerRef.getThreshold(idx) + FloatType(40));
                const auto kneeV = static_cast<float>(
                                       hist.getPercentile(FloatType(0.95)) - hist.getPercentile(FloatType(0.05))
                                   ) / 120.f;
                thresholdUpdater[idx]->update(threshold::convertTo01(threshold::range.snapToLegalValue(thresholdV)));
                kneeUpdater[idx]->update(kneeW::convertTo01(kneeW::range.snapToLegalValue(kneeV)));
            } else {
                controllerRef.setLearningHist(idx, f);
            }
        } else if (parameterID.startsWith(dynamicBypass::ID)) {
            filtersRef[idx].setDynamicBypass(newValue > .5f);
        } else if (parameterID.startsWith(dynamicRelative::ID)) {
            controllerRef.setRelative(idx, newValue > .5f);
        } else if (parameterID.startsWith(targetGain::ID)) {
            value *= static_cast<FloatType>(scale::formatV(parameterRef.getRawParameterValue(scale::ID)->load()));
            value = targetGain::range.snapToLegalValue(static_cast<float>(value));
            controllerRef.getTargetFilter(idx).setGain(value);
        } else if (parameterID.startsWith(targetQ::ID)) {
            controllerRef.getTargetFilter(idx).setQ(value);
            filtersRef[idx].updateIsCurrentDynamicChangeQ();
        } else if (parameterID.startsWith(threshold::ID)) {
            controllerRef.setThreshold(idx, value);
            filtersRef[idx].getCompressor().getComputer().setThreshold(value);
        } else if (parameterID.startsWith(kneeW::ID)) {
            filtersRef[idx].getCompressor().getComputer().setKneeW(kneeW::formatV(value));
        } else if (parameterID.startsWith(sideFreq::ID)) {
            filtersRef[idx].getSideFilter().setFreq(value);
        } else if (parameterID.startsWith(attack::ID)) {
            filtersRef[idx].getCompressor().getDetector().setAttack(value);
        } else if (parameterID.startsWith(release::ID)) {
            filtersRef[idx].getCompressor().getDetector().setRelease(value);
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
