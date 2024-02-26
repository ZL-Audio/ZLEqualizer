// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_SIDE_PANEL_HPP
#define ZLEqualizer_SIDE_PANEL_HPP

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../state/state_definitions.hpp"

namespace zlPanel {
    class SidePanel final : public juce::Component,
                            private juce::AudioProcessorValueTreeState::Listener,
                            private juce::AsyncUpdater {
    public:
        explicit SidePanel(size_t bandIdx,
                           juce::AudioProcessorValueTreeState &parameters,
                           juce::AudioProcessorValueTreeState &parametersNA,
                           zlInterface::UIBase &base,
                           zlDSP::Controller<double> &controller);

        ~SidePanel() override;

        void paint(juce::Graphics &g) override;

    private:
        size_t idx;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        zlIIR::Filter<double> &sideF;
        std::atomic<bool> dynON, selected, actived;

        static constexpr std::array changeIDs{
            zlDSP::dynamicON::ID,
            zlDSP::sideFreq::ID, zlDSP::sideQ::ID
        };

        juce::Colour colour;

        std::atomic<float> scale1{.5f}, scale2{.5f};
        std::atomic<bool> skipRepaint{false};

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;
    };
} // zlPanel

#endif //ZLEqualizer_SIDE_PANEL_HPP
