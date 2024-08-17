// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

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
        }
        parameterNARef.addParameterListener(zlState::maximumDB::ID, this);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::turnOnDynamic(const size_t idx) {
        updateTargetFGQ(idx);
        updateSideFQ(idx);
        const auto paraDynBypass = parameterRef.getParameter(zlDSP::appendSuffix(dynamicBypass::ID, idx));
        updateParaNotifyHost(paraDynBypass, 0.f);
        const auto paraDynLink = parameterRef.getParameter(zlDSP::appendSuffix(singleDynLink::ID, idx));
        updateParaNotifyHost(paraDynLink, static_cast<float>(controllerRef.getDynLink()));
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::turnOffDynamic(const size_t idx) {
        controllerRef.setDynamicON(false, idx);
        const auto paraDynBypass = parameterRef.getParameter(zlDSP::appendSuffix(dynamicBypass::ID, idx));
        updateParaNotifyHost(paraDynBypass, 1.f);
        const auto paraDynLearn = parameterRef.getParameter(zlDSP::appendSuffix(zlDSP::dynamicLearn::ID, idx));
        updateParaNotifyHost(paraDynLearn, 0.f);
        const auto paraDynRelative = parameterRef.getParameter(zlDSP::appendSuffix(dynamicRelative::ID, idx));
        updateParaNotifyHost(paraDynRelative, 0.f);
        const auto paraSideSolo = parameterRef.getParameter(zlDSP::appendSuffix(sideSolo::ID, idx));
        updateParaNotifyHost(paraSideSolo, 0.f);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::updateTargetFGQ(const size_t idx) {
        auto tGain = static_cast<float>(filtersRef[idx].getBaseFilter().getGain());
        switch (filtersRef[idx].getBaseFilter().getFilterType()) {
            case zlIIR::FilterType::peak:
            case zlIIR::FilterType::bandShelf: {
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
            case zlIIR::FilterType::lowShelf:
            case zlIIR::FilterType::highShelf:
            case zlIIR::FilterType::tiltShelf: {
                if (tGain < 0) {
                    tGain += maximumDB.load() * .25f;
                } else {
                    tGain -= maximumDB.load() * .25f;
                }
                break;
            }
            case zlIIR::FilterType::lowPass:
            case zlIIR::FilterType::highPass:
            case zlIIR::FilterType::notch:
            case zlIIR::FilterType::bandPass:
            default: {
                break;
            }
        }
        filtersRef[idx].getTargetFilter().setFreq(filtersRef[idx].getBaseFilter().getFreq(), false);
        filtersRef[idx].getTargetFilter().setFilterType(filtersRef[idx].getBaseFilter().getFilterType(), false);
        filtersRef[idx].getTargetFilter().setOrder(filtersRef[idx].getBaseFilter().getOrder(), true);
        const auto paraGain = parameterRef.getParameter(zlDSP::appendSuffix(targetGain::ID, idx));
        updateParaNotifyHost(paraGain, targetGain::convertTo01(tGain));
        const auto paraQ = parameterRef.getParameter(zlDSP::appendSuffix(targetQ::ID, idx));
        updateParaNotifyHost(paraQ, targetQ::convertTo01(static_cast<float>(filtersRef[idx].getBaseFilter().getQ())));
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::updateSideFQ(const size_t idx) {
        auto [soloFreq, soloQ] = controllerRef.getSoloFilterParas(filtersRef[idx].getBaseFilter());
        const auto soloFreq01 = sideFreq::convertTo01(static_cast<float>(soloFreq));
        const auto soloQ01 = sideQ::convertTo01(static_cast<float>(soloQ));
        const auto paraFreq = parameterRef.getParameter(zlDSP::appendSuffix(sideFreq::ID, idx));
        updateParaNotifyHost(paraFreq, soloFreq01);
        const auto paraQ = parameterRef.getParameter(zlDSP::appendSuffix(sideQ::ID, idx));
        updateParaNotifyHost(paraQ, soloQ01);
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlState::maximumDB::ID) {
            maximumDB.store(zlState::maximumDB::dBs[static_cast<size_t>(newValue)]);
            return;
        }
        const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        auto value = static_cast<FloatType>(newValue);
        if (parameterID.startsWith(bypass::ID)) {
            filtersRef[idx].setBypass(static_cast<bool>(value));
        } else if (parameterID.startsWith(fType::ID)) {
            filtersRef[idx].getBaseFilter().setFilterType(static_cast<zlIIR::FilterType>(value));
            filtersRef[idx].getMainFilter().setFilterType(static_cast<zlIIR::FilterType>(value));
            if (filtersRef[idx].getDynamicON()) {
                filtersRef[idx].getTargetFilter().setFilterType(static_cast<zlIIR::FilterType>(value));
            }
        } else if (parameterID.startsWith(slope::ID)) {
            filtersRef[idx].getBaseFilter().setOrder(slope::orderArray[static_cast<size_t>(value)]);
            filtersRef[idx].getMainFilter().setOrder(slope::orderArray[static_cast<size_t>(value)]);
            if (filtersRef[idx].getDynamicON()) {
                filtersRef[idx].getTargetFilter().setOrder(slope::orderArray[static_cast<size_t>(value)]);
            }
        } else if (parameterID.startsWith(freq::ID)) {
            filtersRef[idx].getBaseFilter().setFreq(value);
            filtersRef[idx].getMainFilter().setFreq(value);
            if (filtersRef[idx].getDynamicON()) {
                filtersRef[idx].getTargetFilter().setFreq(value);
                if (sDynLink[idx].load()) {
                    updateSideFQ(idx);
                }
            }
        } else if (parameterID.startsWith(gain::ID)) {
            value *= static_cast<FloatType>(scale::formatV(parameterRef.getRawParameterValue(scale::ID)->load()));
            value = gain::range.snapToLegalValue(static_cast<float>(value));
            if (filtersRef[idx].getDynamicON()) {
                filtersRef[idx].getBaseFilter().setGain(value);
            } else {
                filtersRef[idx].getBaseFilter().setGain(value);
                filtersRef[idx].getMainFilter().setGain(value);
            }
        } else if (parameterID.startsWith(Q::ID)) {
            if (filtersRef[idx].getDynamicON()) {
                filtersRef[idx].getBaseFilter().setQ(value);
                if (sDynLink[idx].load()) {
                    updateSideFQ(idx);
                }
            } else {
                filtersRef[idx].getBaseFilter().setQ(value);
                filtersRef[idx].getMainFilter().setQ(value);
            }
        } else if (parameterID.startsWith(lrType::ID)) {
            controllerRef.setFilterLRs(static_cast<lrType::lrTypes>(value), idx);
        } else if (parameterID.startsWith(dynamicON::ID)) {
            controllerRef.setDynamicON(static_cast<bool>(value), idx);
        } else if (parameterID.startsWith(dynamicLearn::ID)) {
            const auto f = static_cast<bool>(newValue);
            if (!f && controllerRef.getLearningHistON(idx)) {
                controllerRef.setLearningHist(idx, false);
                const auto &hist = controllerRef.getLearningHist(idx);
                const auto thresholdV = static_cast<float>(-hist.getPercentile(FloatType(0.5)));
                const auto kneeV = static_cast<float>(hist.getPercentile(FloatType(0.95)) -
                                                      hist.getPercentile(FloatType(0.05))) / 120.f;
                const std::array dynamicLearnValues{
                    threshold::convertTo01(threshold::range.snapToLegalValue(thresholdV)),
                    kneeW::convertTo01(kneeW::range.snapToLegalValue(kneeV))
                };
                for (size_t i = 0; i < dynamicLearnIDs.size(); ++i) {
                    auto initID = dynamicLearnIDs[i] + parameterID.getLastCharacters(2);
                    const auto para = parameterRef.getParameter(initID);
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(dynamicLearnValues[i]);
                    para->endChangeGesture();
                }
            } else {
                controllerRef.setLearningHist(idx, f);
            }
        } else if (parameterID.startsWith(dynamicBypass::ID)) {
            filtersRef[idx].setDynamicBypass(static_cast<bool>(value));
        } else if (parameterID.startsWith(dynamicRelative::ID)) {
            controllerRef.setRelative(idx, static_cast<bool>(value));
        } else if (parameterID.startsWith(targetGain::ID)) {
            value *= static_cast<FloatType>(scale::formatV(parameterRef.getRawParameterValue(scale::ID)->load()));
            value = targetGain::range.snapToLegalValue(static_cast<float>(value));
            filtersRef[idx].getTargetFilter().setGain(value);
        } else if (parameterID.startsWith(targetQ::ID)) {
            filtersRef[idx].getTargetFilter().setQ(value);
        } else if (parameterID.startsWith(threshold::ID)) {
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
            sDynLink[idx].store(static_cast<bool>(newValue));
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
