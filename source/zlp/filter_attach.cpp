// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "filter_attach.hpp"

namespace zlp {
    FilterAttach::FilterAttach(juce::AudioProcessor&,
                               juce::AudioProcessorValueTreeState& parameters,
                               Controller& controller, const size_t idx)
        : parameters_(parameters),
          controller_(controller),
          idx_(idx),
          empty_(controller.getEmptyFilters()[idx]),
          side_empty_(controller.getSideEmptyFilters()[idx]),
          dynamic_on_(*parameters.getRawParameterValue(PDynamicON::kID + std::to_string(idx))),
          side_link_(*parameters.getRawParameterValue(PSideLink::kID + std::to_string(idx))),
          side_filter_type_updater_(parameters, PSideFilterType::kID + std::to_string(idx)),
          side_freq_updater_(parameters, PSideFreq::kID + std::to_string(idx)),
          side_Q_updater_(parameters, PSideQ::kID + std::to_string(idx)) {
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameterChanged(kIDs[i], kDefaultVs[i]);
            parameters_.addParameterListener(kIDs[i] + std::to_string(idx_), this);
        }
    }

    FilterAttach::~FilterAttach() {
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameters_.removeParameterListener(kIDs[i] + std::to_string(idx_), this);
        }
    }

    void FilterAttach::parameterChanged(const juce::String& parameter_ID, const float value) {
        if (parameter_ID.startsWith(PFilterStatus::kID)) {
            controller_.setFilterStatus(idx_, static_cast<FilterStatus>(std::round(value)));
        } else if (parameter_ID.startsWith(PFilterType::kID)) {
            empty_.setFilterType(static_cast<zldsp::filter::FilterType>(std::round(value)));
            if (dynamic_on_.load(std::memory_order::relaxed) > .5f &&
                side_link_.load(std::memory_order::relaxed) > .5f) {
                updateSideFilterType();
            }
        } else if (parameter_ID.startsWith(POrder::kID)) {
            empty_.setOrder(POrder::kOrderArray[static_cast<size_t>(std::round(value))]);
        } else if (parameter_ID.startsWith(PLRMode::kID)) {
            controller_.setLRMS(idx_, static_cast<FilterStereo>(std::round(value)));
        } else if (parameter_ID.startsWith(PFreq::kID)) {
            empty_.setFreq(value);
            if (dynamic_on_.load(std::memory_order::relaxed) > .5f &&
                side_link_.load(std::memory_order::relaxed) > .5f) {
                updateSideFreq();
            }
        } else if (parameter_ID.startsWith(PGain::kID)) {
            empty_.setGain(value);
        } else if (parameter_ID.startsWith(PQ::kID)) {
            empty_.setQ(value);
            if (dynamic_on_.load(std::memory_order::relaxed) > .5f &&
                side_link_.load(std::memory_order::relaxed) > .5f) {
                updateSideQ();
            }
        } else if (parameter_ID.startsWith(PDynamicON::kID)) {
            controller_.setDynamicON(idx_, value > .5f);
        } else if (parameter_ID.startsWith(PDynamicBypass::kID)) {
            controller_.setDynamicBypass(idx_, value > .5f);
        } else if (parameter_ID.startsWith(PDynamicLearn::kID)) {
            controller_.setDynamicLearn(idx_, value > .5f);
        } else if (parameter_ID.startsWith(PDynamicRelative::kID)) {
            controller_.setDynamicRelative(idx_, value > .5f);
        } else if (parameter_ID.startsWith(PSideSwap::kID)) {
            controller_.setDynamicSwap(idx_, value > .5f);
        } else if (parameter_ID.startsWith(PSideLink::kID)) {
            if (value > .5f) {
                updateSideFilterType();
                updateSideFreq();
                updateSideQ();
            }
        } else if (parameter_ID.startsWith(PThreshold::kID)) {
            controller_.setDynamicThreshold(idx_, value);
        } else if (parameter_ID.startsWith(PKneeW::kID)) {
            controller_.setDynamicKnee(idx_, value);
        } else if (parameter_ID.startsWith(PAttack::kID)) {
            controller_.setDynamicAttack(idx_, value);
        } else if (parameter_ID.startsWith(PRelease::kID)) {
            controller_.setDynamicRelease(idx_, value);
        } else if (parameter_ID.startsWith(PSideFilterType::kID)) {
            if (value < .5f) {
                side_empty_.setFilterType(zldsp::filter::kBandPass);
            } else if (value < 1.5f) {
                side_empty_.setFilterType(zldsp::filter::kLowPass);
            } else {
                side_empty_.setFilterType(zldsp::filter::kHighPass);
            }
        } else if (parameter_ID.startsWith(PSideFreq::kID)) {
            side_empty_.setFreq(value);
        } else if (parameter_ID.startsWith(PSideQ::kID)) {
            side_empty_.setQ(value);
        } else if (parameter_ID.startsWith(PTargetGain::kID)) {
            side_empty_.setGain(value);
        }
    }

    void FilterAttach::updateSideFilterType() {
        const auto filter_type = empty_.getFilterType();
        if (filter_type == zldsp::filter::kPeak) {
            side_filter_type_updater_.update(0.f);
        } else if (filter_type == zldsp::filter::kLowShelf) {
            side_filter_type_updater_.update(0.5f);
        } else if (filter_type == zldsp::filter::kHighShelf) {
            side_filter_type_updater_.update(1.f);
        }
    }

    void FilterAttach::updateSideFreq() {
        const auto filter_type = empty_.getFilterType();
        if (filter_type == zldsp::filter::kPeak
            || filter_type == zldsp::filter::kLowShelf
            || filter_type == zldsp::filter::kHighShelf) {
            const auto freq = empty_.getFreq();
            side_freq_updater_.update(zlp::PSideFreq::kRange.convertTo0to1(freq));
        }
    }

    void FilterAttach::updateSideQ() {
        if (empty_.getFilterType() == zldsp::filter::kPeak) {
            const auto q = empty_.getQ();
            side_Q_updater_.update(zlp::PSideQ::kRange.convertTo0to1(q));
        }
    }
}
