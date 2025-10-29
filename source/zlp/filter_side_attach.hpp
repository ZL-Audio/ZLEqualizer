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
    class FilterSideAttach final : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit FilterSideAttach(juce::AudioProcessor& processor,
                                  juce::AudioProcessorValueTreeState& parameters,
                                  Controller& controller,
                                  size_t idx);

        ~FilterSideAttach() override;

    private:
        juce::AudioProcessorValueTreeState& parameters_;
        Controller& controller_;
        size_t idx_;
        zldsp::filter::Empty& side_empty_;
        std::atomic<float>& scale_;
        std::atomic<bool>& update_flag_;
        std::atomic<bool>& whole_update_flag_;

        static constexpr std::array kIDs{
            PSideFilterType::kID, PSideOrder::kID, PSideFreq::kID, PSideQ::kID, PTargetGain::kID
        };

        void parameterChanged(const juce::String& parameter_ID, float value) override;
    };
}
