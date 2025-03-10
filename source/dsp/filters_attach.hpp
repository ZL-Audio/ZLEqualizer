// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "controller.hpp"
#include "chore/chore.hpp"
#include "../state/state_definitions.hpp"

namespace zlDSP {
    template<typename FloatType>
    class FiltersAttach : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit FiltersAttach(juce::AudioProcessor &processor,
                               juce::AudioProcessorValueTreeState &parameters,
                               juce::AudioProcessorValueTreeState &parametersNA,
                               Controller<FloatType> &controller);

        ~FiltersAttach() override;

        void addListeners();

        void turnOnDynamic(size_t idx);

        void turnOffDynamic(size_t idx);

        void turnOnDynamicAuto(size_t idx);

        void updateTargetFGQ(size_t idx);

        void updateSideFQ(size_t idx);

    private:
        juce::AudioProcessor &processorRef;
        juce::AudioProcessorValueTreeState &parameterRef, &parameterNARef;
        Controller<FloatType> &controllerRef;
        std::array<zlFilter::DynamicIIR<FloatType, Controller<FloatType>::FilterSize>, bandNUM> &filtersRef;
        std::array<std::string, bandNUM * 2> sideParaNames;
        std::array<std::unique_ptr<zlChore::ParaUpdater>, bandNUM> sideFreqUpdater, sideQUpdater;
        std::array<std::unique_ptr<zlChore::ParaUpdater>, bandNUM> thresholdUpdater, kneeUpdater;

        constexpr static std::array IDs{
            bypass::ID, fType::ID, slope::ID, freq::ID, gain::ID, Q::ID,
            lrType::ID, dynamicON::ID, dynamicLearn::ID,
            dynamicBypass::ID, dynamicRelative::ID, sideSwap::ID,
            targetGain::ID, targetQ::ID, threshold::ID, kneeW::ID,
            sideFreq::ID, attack::ID, release::ID, sideQ::ID,
            singleDynLink::ID
        };

        constexpr static std::array defaultVs{
            float(bypass::defaultV), float(fType::defaultI), float(slope::defaultI),
            freq::defaultV, gain::defaultV, Q::defaultV,
            float(lrType::defaultI), float(dynamicON::defaultV), float(dynamicLearn::defaultV),
            float(dynamicBypass::defaultV), float(dynamicRelative::defaultV), float(sideSwap::defaultV),
            targetGain::defaultV, targetQ::defaultV,
            threshold::defaultV, kneeW::defaultV,
            sideFreq::defaultV, attack::defaultV, release::defaultV, sideQ::defaultV,
            float(singleDynLink::defaultV)
        };

        constexpr static std::array dynamicInitIDs{
            targetGain::ID, targetQ::ID, sideFreq::ID, sideQ::ID,
            dynamicBypass::ID, singleDynLink::ID
        };
        constexpr static std::array dynamicResetIDs{
            dynamicLearn::ID, dynamicBypass::ID,
            sideSolo::ID, dynamicRelative::ID
        };

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void initDefaultValues();

        std::atomic<float> maximumDB{zlState::maximumDB::dBs[static_cast<size_t>(zlState::maximumDB::defaultI)]};

        std::atomic<bool> gDynLink{false};
        std::array<std::atomic<bool>, bandNUM> sDynLink{};
    };
}
