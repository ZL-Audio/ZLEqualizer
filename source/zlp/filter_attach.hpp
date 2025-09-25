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

namespace zlp {
    class FilterAttach final : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit FilterAttach(juce::AudioProcessor& processor,
                              juce::AudioProcessorValueTreeState& parameters,
                              Controller& controller,
                              size_t idx);

        ~FilterAttach() override;

    private:
        juce::AudioProcessorValueTreeState& parameters_;
        Controller& controller_;
        size_t idx_;
        zldsp::filter::Empty& empty_;
        zldsp::filter::Empty& side_empty_;
        static constexpr std::array kIDs{
            PFilterStatus::kID, PFilterType::kID, POrder::kID, PLRMode::kID,
            PFreq::kID, PGain::kID, PQ::kID,
            PDynamicON::kID, PDynamicBypass::kID,
            PDynamicLearn::kID, PDynamicRelative::kID, PSideSwap::kID,
            PThreshold::kID, PKneeW::kID, PAttack::kID, PRelease::kID,
            PSideFreq::kID, PSideQ::kID, PTargetGain::kID
        };
        static constexpr std::array kDefaultVs{
            static_cast<float>(PFilterStatus::kDefaultI),
            static_cast<float>(PFilterType::kDefaultI),
            static_cast<float>(POrder::kDefaultI),
            static_cast<float>(PLRMode::kDefaultI),
            PFreq::kDefaultV, PGain::kDefaultV, PQ::kDefaultV,
            static_cast<float>(PDynamicON::kDefaultV),
            static_cast<float>(PDynamicBypass::kDefaultV),
            static_cast<float>(PDynamicLearn::kDefaultV),
            static_cast<float>(PDynamicRelative::kDefaultV),
            static_cast<float>(PSideSwap::kDefaultV),
            PThreshold::kDefaultV, PKneeW::kDefaultV, PAttack::kDefaultV, PRelease::kDefaultV,
            PSideFreq::kDefaultV, PSideQ::kDefaultV, PTargetGain::kDefaultV
        };

        void parameterChanged(const juce::String& parameter_ID, float value) override;
    };
} // zlp
