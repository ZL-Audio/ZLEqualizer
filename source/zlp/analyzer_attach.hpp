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
#include "../state/state_definitions.hpp"

namespace zlp {
    class AnalyzerAttach final : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit AnalyzerAttach(juce::AudioProcessor& processor,
                                juce::AudioProcessorValueTreeState& parameters_NA,
                                Controller& controller);

        ~AnalyzerAttach() override;

    private:
        juce::AudioProcessorValueTreeState& parameters_NA_;
        Controller& controller_;

        static constexpr std::array kIDs{
            zlstate::PFFTPreON::kID, zlstate::PFFTPostON::kID, zlstate::PFFTSideON::kID,
            zlstate::PFFTSpeed::kID, zlstate::PFFTTilt::kID
        };

        void parameterChanged(const juce::String& parameter_ID, float value) override;
    };
}
