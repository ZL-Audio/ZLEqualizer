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
        explicit SidePanel(size_t bandIdx,
                           juce::AudioProcessorValueTreeState &parameters,
                           juce::AudioProcessorValueTreeState &parameters_NA,
                           zlgui::UIBase &base,
                           zlp::Controller<double> &controller,
                           zlgui::Dragger &sideDragger);

        ~SidePanel() override;

        void paint(juce::Graphics &g) override;

        void lookAndFeelChanged() override;

        void updateDragger();

    private:
        size_t idx;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &uiBase;
        zldsp::filter::IIR<double, zlp::Controller<double>::kFilterSize> &sideF;
        zlgui::Dragger &sideDraggerRef;
        std::atomic<bool> dynON, selected, actived;

        static constexpr std::array changeIDs{
            zlp::dynamicON::ID, zlp::sideQ::ID
        };

        juce::Colour colour;

        std::atomic<double> sideQ{0.707};
        std::atomic<bool> toUpdate{false};
        float currentBW{0.f};

        void parameterChanged(const juce::String &parameterID, float newValue) override;
    };
} // zlpanel
