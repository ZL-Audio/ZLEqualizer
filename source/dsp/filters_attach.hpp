// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_FILTERS_ATTACH_HPP
#define ZLEQUALIZER_FILTERS_ATTACH_HPP

#include "controller.hpp"
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

        inline void enableDynamicONUpdateOthers(const bool x) { dynamicONUpdateOthers.store(x); }

    private:
        juce::AudioProcessor &processorRef;
        juce::AudioProcessorValueTreeState &parameterRef, &parameterNARef;
        Controller<FloatType> &controllerRef;
        std::array<zlDynamicFilter::IIRFilter<FloatType>, bandNUM> &filtersRef;

        constexpr static std::array IDs{
            bypass::ID, fType::ID, slope::ID, freq::ID, gain::ID, Q::ID,
            lrType::ID, dynamicON::ID, dynamicLearn::ID,
            dynamicBypass::ID, dynamicRelative::ID,
            targetGain::ID, targetQ::ID, threshold::ID, kneeW::ID,
            sideFreq::ID, attack::ID, release::ID, sideQ::ID
        };

        constexpr static std::array defaultVs{
            float(bypass::defaultV), float(fType::defaultI), float(slope::defaultI),
            freq::defaultV, gain::defaultV, Q::defaultV,
            float(lrType::defaultI), float(dynamicON::defaultV), float(dynamicLearn::defaultV),
            float(dynamicBypass::defaultV), float(dynamicRelative::defaultV),
            targetGain::defaultV, targetQ::defaultV,
            threshold::defaultV, kneeW::defaultV,
            sideFreq::defaultV, attack::defaultV, release::defaultV, sideQ::defaultV
        };

        constexpr static std::array dynamicInitIDs{
            targetGain::ID, targetQ::ID, sideFreq::ID, sideQ::ID,
            dynamicBypass::ID
        };
        constexpr static std::array dynamicResetIDs{dynamicLearn::ID, dynamicBypass::ID,
            sideSolo::ID, dynamicRelative::ID};

        constexpr static std::array dynamicLearnIDs {threshold::ID, kneeW::ID};

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void initDefaultValues();

        std::atomic<bool> dynamicONUpdateOthers = true;
        std::atomic<float> maximumDB{zlState::maximumDB::dBs[static_cast<size_t>(zlState::maximumDB::defaultI)]};
    };
}

#endif //ZLEQUALIZER_FILTERS_ATTACH_HPP
