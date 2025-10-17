// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "dragger_panel.hpp"

namespace zlpanel {
    class ButtonPanel final : public juce::Component {
    public:
        explicit ButtonPanel(PluginProcessor& p, zlgui::UIBase& base,
                             const multilingual::TooltipHelper& tooltip_helper);

        void mouseDoubleClick(const juce::MouseEvent& event) override;

        void updateBand();

        void updateSampleRate(double sample_rate);

    private:
        static constexpr std::array kInitIDs{
            zlp::PFilterStatus::kID, zlp::PFilterType::kID,
            zlp::POrder::kID,
            zlp::PFreq::kID, zlp::PGain::kID, zlp::PQ::kID,
            zlp::PDynamicON::kID
        };

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;

        float fft_max_{0.f};
        float slider_max_{0.f};
    };
}
