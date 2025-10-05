// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "sub_left_control_panel.hpp"

namespace zlpanel {
    class LeftControlPanel final : public juce::Component {
    public:
        explicit LeftControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                  const multilingual::TooltipHelper& tooltip_helper);

        ~LeftControlPanel() override;

        int getIdealWidth() const;

        void resized() override;

        void repaintCallBackSlow();

        void updateBand();

        void updateFreqMax(double freq_max);

        void turnOnOffDynamic(bool dynamic_on);

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_;

        SubLeftControlPanel sub_left_control_panel_;

        zlgui::slider::TwoValueRotarySlider<true, false, false> freq_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> freq_attachment_;
        double freq_max_{20000.0};

        zlgui::slider::TwoValueRotarySlider<true, true, false> gain_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> gain_attachment_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> target_gain_attachment_;
    };
} // zlpanel
