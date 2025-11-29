// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "filter_dynamic_attach.hpp"

namespace zlp {
    FilterDynamicAttach::FilterDynamicAttach(juce::AudioProcessor&,
                                             juce::AudioProcessorValueTreeState& parameters,
                                             Controller& controller, const size_t idx) :
        parameters_(parameters),
        controller_(controller),
        idx_(idx) {
        for (size_t i = 0; i < kIDs.size(); ++i) {
            const auto ID = kIDs[i] + std::to_string(idx_);
            parameters_.addParameterListener(ID, this);
            parameterChanged(ID, parameters.getRawParameterValue(ID)->load(std::memory_order::relaxed));
        }
    }

    FilterDynamicAttach::~FilterDynamicAttach() {
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameters_.removeParameterListener(kIDs[i] + std::to_string(idx_), this);
        }
    }

    void FilterDynamicAttach::parameterChanged(const juce::String& parameter_ID, const float value) {
        if (parameter_ID.startsWith(PDynamicON::kID)) {
            controller_.setDynamicON(idx_, value > .5f);
        } else if (parameter_ID.startsWith(PDynamicBypass::kID)) {
            controller_.setDynamicBypass(idx_, value > .5f);
        } else if (parameter_ID.startsWith(PDynamicLearn::kID)) {
            controller_.setDynamicLearn(idx_, value > .5f);
        } else if (parameter_ID.startsWith(PDynamicRelative::kID)) {
            controller_.setDynamicRelative(idx_, value > .5f);
        } else if (parameter_ID.startsWith(PSideSwap::kID)) {
            controller_.setDynamicSwap(idx_, value > .5f);
        } else if (parameter_ID.startsWith(PThreshold::kID)) {
            controller_.setDynamicThreshold(idx_, value);
        } else if (parameter_ID.startsWith(PKneeW::kID)) {
            controller_.setDynamicKnee(idx_, value);
        } else if (parameter_ID.startsWith(PAttack::kID)) {
            controller_.setDynamicAttack(idx_, value);
        } else if (parameter_ID.startsWith(PRelease::kID)) {
            controller_.setDynamicRelease(idx_, value);
        } else if (parameter_ID.startsWith(PDynamicSmooth::kID)) {
            controller_.setDynamicSmooth(idx_, value * 0.01f);
        }
    }
}
