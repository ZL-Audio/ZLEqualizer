// Copyright (C) 2023 - zsliu98
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
                                            Controller<FloatType> &controller)
            :processorRef(processor), parameterRef(parameters),
             controllerRef(controller), filtersRef(controller.getFilters()) {
        addListeners();
        initDefaultValues();
    }

    template<typename FloatType>
    FiltersAttach<FloatType>::~FiltersAttach() {
        for (int i = 0; i < bandNUM; ++i) {
            auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            for (auto &ID: IDs) {
                parameterRef.removeParameterListener(ID + suffix, this);
            }
        }
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::addListeners() {
        for (int i = 0; i < bandNUM; ++i) {
            auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            for (auto &ID: IDs) {
                parameterRef.addParameterListener(ID + suffix, this);
            }
        }
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::parameterChanged(const juce::String &parameterID, float newValue) {
        auto id = parameterID.dropLastCharacters(2);
        auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        auto value = static_cast<FloatType>(newValue);
//        logger.logMessage(id + " " + juce::String(idx));
        if (id == bypass::ID) {
            filtersRef[idx].setBypass(static_cast<bool>(value));
        } else if (id == fType::ID) {
            filtersRef[idx].getBaseFilter().setFilterType(static_cast<zlIIR::FilterType>(value));
            filtersRef[idx].getMainFilter().setFilterType(static_cast<zlIIR::FilterType>(value));
            filtersRef[idx].getTargetFilter().setFilterType(static_cast<zlIIR::FilterType>(value));
        } else if (id == slope::ID) {
            filtersRef[idx].getBaseFilter().setOrder(slope::orderArray[static_cast<size_t>(value)]);
            filtersRef[idx].getMainFilter().setOrder(slope::orderArray[static_cast<size_t>(value)]);
            filtersRef[idx].getTargetFilter().setOrder(slope::orderArray[static_cast<size_t>(value)]);
        } else if (id == freq::ID) {
            filtersRef[idx].getBaseFilter().setFreq(value);
            filtersRef[idx].getMainFilter().setFreq(value);
            filtersRef[idx].getTargetFilter().setFreq(value);
        } else if (id == gain::ID) {
            filtersRef[idx].getBaseFilter().setGain(value);
            filtersRef[idx].getMainFilter().setGain(value);
        } else if (id == Q::ID) {
            filtersRef[idx].getBaseFilter().setQ(value);
            filtersRef[idx].getMainFilter().setQ(value);
        } else if (id == lrType::ID) {
            controllerRef.setFilterLRs(static_cast<lrTypes>(value), idx);
        } else if (id == dynamicON::ID) {
            if (!filtersRef[idx].getDynamicON() && static_cast<bool>(value) && dynamicONUpdateOthers.load()) {
                std::array dynamicInitValues{targetGain::range.convertTo0to1(
                                                     static_cast<float>(filtersRef[idx].getBaseFilter().getGain())),
                                             targetQ::range.convertTo0to1(
                                                     static_cast<float>(filtersRef[idx].getBaseFilter().getQ())),
                                             sideFreq::range.convertTo0to1(
                                                     static_cast<float>(filtersRef[idx].getSideDefaultFreq())),
                                                     static_cast<float>(false)};
                for (size_t i = 0; i < dynamicInitIDs.size(); ++i) {
                    auto initID = dynamicInitIDs[i] + parameterID.getLastCharacters(2);
                    parameterRef.getParameter(initID)->beginChangeGesture();
                    parameterRef.getParameter(initID)->setValueNotifyingHost(dynamicInitValues[i]);
                    parameterRef.getParameter(initID)->endChangeGesture();
                }
            }
            controllerRef.setDynamicON(static_cast<bool>(value), idx);
        } else if (id == dynamicBypass::ID) {
//            logger.logMessage("set dynamic bypass to " + juce::String(value));
            filtersRef[idx].setDynamicBypass(static_cast<bool>(value));
        } else if (id == targetGain::ID) {
//            logger.logMessage("set target gain to " + juce::String(value));
            filtersRef[idx].getTargetFilter().setGain(value);
        } else if (id == targetQ::ID) {
            filtersRef[idx].getTargetFilter().setQ(value);
        } else if (id == threshold::ID) {
            filtersRef[idx].getCompressor().getComputer().setThreshold(value);
        } else if (id == ratio::ID) {
            filtersRef[idx].getCompressor().getComputer().setRatio(value);
        } else if (id == sideFreq::ID) {
            filtersRef[idx].getSideFilter().setFreq(value);
        } else if (id == attack::ID) {
            filtersRef[idx].getCompressor().getDetector().setAttack(value);
        } else if (id == release::ID) {
            filtersRef[idx].getCompressor().getDetector().setRelease(value);
        } else if (id == sideQ::ID) {
            filtersRef[idx].getSideFilter().setQ(value);
        }
    }

    template<typename FloatType>
    void FiltersAttach<FloatType>::initDefaultValues() {
        enableDynamicONUpdateOthers(false);
        for (int i = 0; i < bandNUM; ++i) {
            auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            for (size_t j = 0; j < defaultVs.size(); ++j) {
                parameterChanged(IDs[j] + suffix, defaultVs[j]);
            }
        }
        enableDynamicONUpdateOthers(true);
    }

    template
    class FiltersAttach<float>;

    template
    class FiltersAttach<double>;
}