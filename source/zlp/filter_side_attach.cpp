// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "filter_side_attach.hpp"

namespace zlp {
    FilterSideAttach::FilterSideAttach(juce::AudioProcessor&,
                               juce::AudioProcessorValueTreeState& parameters,
                               Controller& controller, const size_t idx)
        : parameters_(parameters),
          controller_(controller),
          idx_(idx),
          side_empty_(controller.getSideEmptyFilters()[idx]) {
        juce::ignoreUnused(controller_);
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameterChanged(kIDs[i], kDefaultVs[i]);
            parameters_.addParameterListener(kIDs[i] + std::to_string(idx_), this);
        }
    }

    FilterSideAttach::~FilterSideAttach() {
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameters_.removeParameterListener(kIDs[i] + std::to_string(idx_), this);
        }
    }

    void FilterSideAttach::parameterChanged(const juce::String& parameter_ID, const float value) {
        if (parameter_ID.startsWith(PSideFilterType::kID)) {
            if (value < .5f) {
                side_empty_.setFilterType(zldsp::filter::kBandPass);
            } else if (value < 1.5f) {
                side_empty_.setFilterType(zldsp::filter::kLowPass);
            } else {
                side_empty_.setFilterType(zldsp::filter::kHighPass);
            }
        } else if (parameter_ID.startsWith(PSideOrder::kID)) {
            side_empty_.setOrder(PSideOrder::kOrderArray[static_cast<size_t>(std::round(value))]);
        } else if (parameter_ID.startsWith(PSideFreq::kID)) {
            side_empty_.setFreq(value);
        } else if (parameter_ID.startsWith(PSideQ::kID)) {
            side_empty_.setQ(value);
        } else if (parameter_ID.startsWith(PTargetGain::kID)) {
            side_empty_.setGain(value);
        }
    }
}
