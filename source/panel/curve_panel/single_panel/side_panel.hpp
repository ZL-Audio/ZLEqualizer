// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_SIDE_PANEL_HPP
#define ZLEqualizer_SIDE_PANEL_HPP

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../state/state_definitions.hpp"

namespace zlPanel {
    class SidePanel final : public juce::Component,
                            private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit SidePanel(size_t bandIdx,
                           juce::AudioProcessorValueTreeState &parameters,
                           juce::AudioProcessorValueTreeState &parametersNA,
                           zlInterface::UIBase &base,
                           zlDSP::Controller<double> &controller);

        ~SidePanel() override;

        void paint(juce::Graphics &g) override;

        bool checkRepaint();

        void lookAndFeelChanged() override;

    private:
        size_t idx;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        zlFilter::IIR<double, zlDSP::Controller<double>::FilterSize> &sideF;
        std::atomic<bool> dynON, selected, actived;
        std::atomic<bool> toRepaint{false};

        static constexpr std::array changeIDs{
            zlDSP::dynamicON::ID, zlDSP::sideFreq::ID, zlDSP::sideQ::ID
        };

        juce::Colour colour;

        std::atomic<double> sideFreq{1000.0}, sideQ{0.707};
        std::atomic<float> scale1{.5f}, scale2{.5f};
        std::atomic<bool> skipRepaint{false};
        std::atomic<bool> toUpdate{false};

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void update();
    };
} // zlPanel

#endif //ZLEqualizer_SIDE_PANEL_HPP
