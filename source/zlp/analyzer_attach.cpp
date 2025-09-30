// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "analyzer_attach.hpp"

namespace zlp {
    AnalyzerAttach::AnalyzerAttach(juce::AudioProcessor&,
                                   juce::AudioProcessorValueTreeState& parameters_NA,
                                   Controller& controller)
        : parameters_NA_(parameters_NA), controller_(controller) {
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameters_NA.addParameterListener(kIDs[i], this);
            parameterChanged(kIDs[i],
                             parameters_NA.getRawParameterValue(kIDs[i])->load(std::memory_order::relaxed));
        }
    }

    AnalyzerAttach::~AnalyzerAttach() {
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameters_NA_.removeParameterListener(kIDs[i], this);
        }
    }

    void AnalyzerAttach::parameterChanged(const juce::String& parameter_ID, const float value) {
        if (parameter_ID == zlstate::PFFTPreON::kID) {
            controller_.getFFTAnalyzer().setON(0, value > .5f);
        } else if (parameter_ID == zlstate::PFFTPostON::kID) {
            controller_.getFFTAnalyzer().setON(1, value > .5f);
        } else if (parameter_ID == zlstate::PFFTSideON::kID) {
            controller_.getFFTAnalyzer().setON(2, value > .5f);
        } else if (parameter_ID == zlstate::PFFTSpeed::kID) {
            const auto speed = zlstate::PFFTSpeed::kSpeeds[static_cast<size_t>(std::round(value))];
            controller_.getFFTAnalyzer().setDecayRate(0, speed);
            controller_.getFFTAnalyzer().setDecayRate(1, speed);
            controller_.getFFTAnalyzer().setDecayRate(2, speed);
        } else if (parameter_ID == zlstate::PFFTTilt::kID) {
            const auto slope = zlstate::PFFTTilt::kSlopes[static_cast<size_t>(std::round(value))];
            controller_.getFFTAnalyzer().setTiltSlope(slope);
        }
    }
}
