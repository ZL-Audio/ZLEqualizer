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
#include "../../../multilingual/tooltip_helper.hpp"

#include "../../../control_panel/control_background.hpp"

namespace zlpanel {
    class RightClickPanel final : public juce::Component {
    public:
        explicit RightClickPanel(PluginProcessor& p, zlgui::UIBase& base,
                                 const multilingual::TooltipHelper& tooltip_helper);

        int getIdealWidth() const;

        int getIdealHeight() const;

        void resized() override;

        void setPosition(juce::Point<float> pos);

        void setSafeArea(juce::Rectangle<float> area);

    private:
        static constexpr std::array kIDs{
            zlp::PFilterStatus::kID,
            zlp::PFilterType::kID,
            zlp::POrder::kID, zlp::PLRMode::kID,
            zlp::PFreq::kID,
            zlp::PGain::kID, zlp::PTargetGain::kID,
            zlp::PQ::kID,
            zlp::PDynamicON::kID, zlp::PDynamicLearn::kID,
            zlp::PDynamicBypass::kID, zlp::PDynamicRelative::kID,
            zlp::PSideSwap::kID,
            zlp::PSideLink::kID,
            zlp::PThreshold::kID, zlp::PKneeW::kID, zlp::PAttack::kID, zlp::PRelease::kID,
            zlp::PSideFilterType::kID, zlp::PSideOrder::kID,
            zlp::PSideFreq::kID, zlp::PSideQ::kID
        };

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        juce::SelectedItemSet<size_t>& items_set_;
        ControlBackground control_background_;

        juce::Rectangle<float> safe_area_{};

        juce::Component mouse_event_eater_;
        zlgui::button::ClickTextButton invert_button_;
        zlgui::button::ClickTextButton lr_split_button_;
        zlgui::button::ClickTextButton ms_split_button_;
        zlgui::button::ClickTextButton copy_button_;
        zlgui::button::ClickTextButton paste_button_;

        void visibilityChanged() override;

        void invertGain();

        void splitBand(zlp::FilterStereo stereo1, zlp::FilterStereo stereo2);

        void copyBand();

        void pasteBand();
    };
}
