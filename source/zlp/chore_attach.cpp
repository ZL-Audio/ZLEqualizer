// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "chore_attach.hpp"

namespace zlp {
    ChoreAttach::ChoreAttach(juce::AudioProcessor&,
                             juce::AudioProcessorValueTreeState& parameters,
                             Controller& controller) :
        parameters_(parameters), controller_(controller),
        output_gain_updater_(parameters, POutputGain::kID),
        agc_updater_(parameters, PAutoGain::kID) {
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameters_.addParameterListener(kIDs[i], this);
            parameterChanged(kIDs[i],
                             parameters.getRawParameterValue(kIDs[i])->load(std::memory_order::relaxed));
        }
    }

    ChoreAttach::~ChoreAttach() {
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameters_.removeParameterListener(kIDs[i], this);
        }
    }

    void ChoreAttach::parameterChanged(const juce::String& parameter_ID, const float value) {
        if (parameter_ID == PFilterStructure::kID) {
            controller_.setFilterStructure(static_cast<FilterStructure>(std::round(value)));
        } else if (parameter_ID == POutputGain::kID) {
            controller_.setMakeupGain(value);
        } else if (parameter_ID == PStaticGain::kID) {
            controller_.setSGCON(value > .5f);
        } else if (parameter_ID == PMakeupLearn::kID) {
            if (value > .5f) {
                controller_.setLoudnessMatchON(true);
            } else {
                controller_.setLoudnessMatchON(false);
                const auto c_diff = controller_.getLUFSMatcherDiff();
                output_gain_updater_.update(-static_cast<float>(c_diff));
                agc_updater_.update(0.f);
            }
        } else if (parameter_ID == PAutoGain::kID) {
            controller_.setAGCON(value > .5f);
        } else if (parameter_ID == PPhaseFlip::kID) {
            controller_.setPhaseFlipON(value > .5f);
        } else if (parameter_ID == PLookahead::kID) {
            controller_.setDelay(static_cast<double>(value) * 0.001);
        }
    }
}
