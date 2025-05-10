// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../panel_definitons.hpp"

namespace zlpanel {
    class LoudnessDisplay final : public juce::Component {
    public:
        explicit LoudnessDisplay(PluginProcessor &p, zlgui::UIBase &base);

        void paint(juce::Graphics &g) override;

        void checkVisible();

        void lookAndFeelChanged() override;

        void updateVisible(bool x);

    private:
        PluginProcessor &processor_ref_;
        zlgui::UIBase &ui_base_;
        juce::Time previous_time_{};

        size_t band_idx_{0};
        std::array<juce::RangedAudioParameter *, zlstate::kBandNUM> is_dynamic_on_paras_{};
        std::array<juce::RangedAudioParameter *, zlstate::kBandNUM> is_threshold_auto_paras_{};
        juce::RangedAudioParameter *band_idx_para_;
        juce::Colour colour_;
        bool should_visible_{false};
    };
} // zlpanel
