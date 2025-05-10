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

namespace zlp {
    template<typename FloatType>
    class FiltersAttach : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit FiltersAttach(juce::AudioProcessor &processor,
                               juce::AudioProcessorValueTreeState &parameters,
                               juce::AudioProcessorValueTreeState &parameters_NA,
                               Controller<FloatType> &controller);

        ~FiltersAttach() override;

        void addListeners();

        void turnOnDynamic(size_t idx);

        void turnOffDynamic(size_t idx);

        void turnOnDynamicAuto(size_t idx);

        void updateTargetFGQ(size_t idx);

        void updateSideFQ(size_t idx);

    private:
        juce::AudioProcessor &processor_ref_;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        Controller<FloatType> &controller_ref_;
        std::array<zldsp::filter::DynamicIIR<FloatType, Controller<FloatType>::kFilterSize>, kBandNUM> &filters_ref_;
        std::array<std::string, kBandNUM * 2> side_para_names_;
        std::array<std::unique_ptr<zldsp::chore::ParaUpdater>, kBandNUM> side_freq_updater_, side_q_updater_;
        std::array<std::unique_ptr<zldsp::chore::ParaUpdater>, kBandNUM> threshold_updater_, knee_updater_;

        constexpr static std::array kIDs{
            bypass::ID, fType::ID, slope::ID, freq::ID, gain::ID, Q::ID,
            lrType::ID, dynamicON::ID, dynamicLearn::ID,
            dynamicBypass::ID, dynamicRelative::ID, sideSwap::ID,
            targetGain::ID, targetQ::ID, threshold::ID, kneeW::ID,
            sideFreq::ID, attack::ID, release::ID, sideQ::ID,
            singleDynLink::ID
        };

        constexpr static std::array kDefaultVs{
            float(bypass::defaultV), float(fType::defaultI), float(slope::defaultI),
            freq::defaultV, gain::defaultV, Q::defaultV,
            float(lrType::defaultI), float(dynamicON::defaultV), float(dynamicLearn::defaultV),
            float(dynamicBypass::defaultV), float(dynamicRelative::defaultV), float(sideSwap::defaultV),
            targetGain::defaultV, targetQ::defaultV,
            threshold::defaultV, kneeW::defaultV,
            sideFreq::defaultV, attack::defaultV, release::defaultV, sideQ::defaultV,
            float(singleDynLink::defaultV)
        };

        constexpr static std::array kDynamicInitIDs{
            targetGain::ID, targetQ::ID, sideFreq::ID, sideQ::ID,
            dynamicBypass::ID, singleDynLink::ID
        };
        constexpr static std::array kDynamicResetIDs{
            dynamicLearn::ID, dynamicBypass::ID,
            sideSolo::ID, dynamicRelative::ID
        };

        void parameterChanged(const juce::String &parameter_id, float new_value) override;

        void initDefaultValues();

        std::atomic<float> maximum_db_{zlstate::maximumDB::dBs[static_cast<size_t>(zlstate::maximumDB::defaultI)]};

        std::atomic<bool> g_dyn_link_{false};
        std::array<std::atomic<bool>, kBandNUM> s_dyn_link_{};
    };
}
