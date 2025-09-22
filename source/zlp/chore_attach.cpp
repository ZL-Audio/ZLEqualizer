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
                             Controller& controller)
        : parameters_(parameters), controller_(controller) {
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameterChanged(kIDs[i], kDefaultVs[i]);
            parameters_.addParameterListener(kIDs[i], this);
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
        }
    }
}
