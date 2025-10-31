// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../../PluginProcessor.hpp"
#include "../../../../gui/gui.hpp"
#include "../../../helper/helper.hpp"

namespace zlpanel {
    class LassoBandUpdater final : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit LassoBandUpdater(PluginProcessor& p, zlgui::UIBase& base);

        ~LassoBandUpdater() override;

        void updateBand();

        void loadParas();

        void repaintCallBack();

    private:
        static constexpr std::array kScaleIDs{
            zlp::PFreq::kID,
            zlp::PGain::kID, zlp::PTargetGain::kID,
            zlp::PQ::kID,
            zlp::PAttack::kID, zlp::PRelease::kID
        };

        static constexpr std::array kSyncIDs{
            zlp::PFilterStatus::kID,
            zlp::POrder::kID, zlp::PLRMode::kID,
            zlp::PDynamicBypass::kID,
            zlp::PKneeW::kID
        };

        static constexpr std::array kShiftIDs{
            zlp::PThreshold::kID, zlp::PKneeW::kID
        };

        std::atomic<bool> whole_to_update_{false};
        std::atomic<bool> whole_to_update_scale_{false};
        std::atomic<bool> whole_to_update_sync_{false};
        std::atomic<bool> whole_to_update_shift_{false};
        std::array<std::atomic<bool>, kScaleIDs.size()> to_update_scale_{};
        std::array<std::atomic<bool>, kSyncIDs.size()> to_update_sync_{};
        std::array<std::atomic<bool>, kShiftIDs.size()> to_update_shift_{};

        std::array<std::array<juce::RangedAudioParameter*, zlp::kBandNum>, kScaleIDs.size()> scale_paras_;
        std::array<std::array<juce::RangedAudioParameter*, zlp::kBandNum>, kSyncIDs.size()> sync_paras_;
        std::array<std::array<juce::RangedAudioParameter*, zlp::kBandNum>, kShiftIDs.size()> shift_paras_;

        std::array<std::array<float, zlp::kBandNum>, kScaleIDs.size()> scale_values_when_selected_{};
        std::array<std::array<float, zlp::kBandNum>, kShiftIDs.size()> shift_values_when_selected_{};

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        juce::SelectedItemSet<size_t>& items_set_;
        size_t previous_band_{zlp::kBandNum};

        template <bool add>
        void listenerAddRemove(size_t band);

        void parameterChanged(const juce::String& parameter_ID, float value) override;

        void updateScaleParas();

        void updateSyncParas();

        void updateShiftParas();
    };
}
