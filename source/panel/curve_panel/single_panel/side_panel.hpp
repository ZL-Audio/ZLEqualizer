// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../state/state_definitions.hpp"

namespace zlpanel {
    class SidePanel final : public juce::Component,
                            private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit SidePanel(size_t band_idx,
                           juce::AudioProcessorValueTreeState &parameters,
                           juce::AudioProcessorValueTreeState &parameters_NA,
                           zlgui::UIBase &base,
                           zlp::Controller<double> &controller,
                           zlgui::Dragger &side_dragger);

        ~SidePanel() override;

        void paint(juce::Graphics &g) override;

        void lookAndFeelChanged() override;

        void updateDragger();

    private:
        size_t band_idx_;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        zldsp::filter::IIR<double, zlp::Controller<double>::kFilterSize> &side_f_;
        zlgui::Dragger &side_dragger_ref_;
        std::atomic<bool> dyn_on_, selected_, active_;

        static constexpr std::array kChangeIDs{
            zlp::dynamicON::ID, zlp::sideQ::ID
        };

        juce::Colour colour_;

        std::atomic<double> side_q_{0.707};
        std::atomic<bool> to_update_{false};
        float current_bw_{0.f};

        void parameterChanged(const juce::String &parameter_id, float new_value) override;
    };
} // zlpanel
