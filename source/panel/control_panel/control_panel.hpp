// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "left_control_panel/left_control_panel.hpp"
#include "right_control_panel/right_control_panel.hpp"
#include "match_control_panel/match_control_panel.hpp"

namespace zlpanel {
    class ControlPanel final : public juce::Component,
                               private juce::AudioProcessorValueTreeState::Listener,
                               private juce::AsyncUpdater {
    public:
        explicit ControlPanel(PluginProcessor &p,
                              zlgui::UIBase &base);

        ~ControlPanel() override;

        void resized() override;

        void paint(juce::Graphics &g) override;

    private:
        juce::AudioProcessorValueTreeState &parameters_ref_;
        juce::AudioProcessorValueTreeState &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        std::atomic<size_t> band_idx_{0};
        std::array<std::atomic<bool>, zlstate::kBandNUM> dynamic_on_{};
        LeftControlPanel left_control_panel_;
        RightControlPanel right_control_panel_;
        MatchControlPanel match_control_panel_;

        void parameterChanged(const juce::String &parameter_id, float new_value) override;

        void handleAsyncUpdate() override;
    };
} // zlpanel
