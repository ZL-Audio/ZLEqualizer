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
#include "juce_helper/para_updater.hpp"

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
        std::atomic<float>& scale_;
        std::atomic<bool>& update_flag_;
        std::atomic<bool>& whole_update_flag_;

        std::atomic<float>& side_link_;

        juce_helper::ParaUpdater side_filter_type_updater_;
        juce_helper::ParaUpdater side_freq_updater_;
        juce_helper::ParaUpdater side_Q_updater_;

        static constexpr std::array kIDs{
            PFilterStatus::kID, PFilterType::kID, POrder::kID, PLRMode::kID,
            PFreq::kID, PGain::kID, PQ::kID,
            PSideLink::kID,
        };

        void parameterChanged(const juce::String& parameter_ID, float value) override;

        void updateSideFilterType();

        void updateSideFreq();

        void updateSideQ();
    };
} // zlp
