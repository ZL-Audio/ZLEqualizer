// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_SOLO_PANEL_HPP
#define ZLEqualizer_SOLO_PANEL_HPP

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"

namespace zlPanel {
    class SoloPanel final : public juce::Component,
                            private juce::AudioProcessorValueTreeState::Listener,
                            private juce::AsyncUpdater {
    public:
        SoloPanel(juce::AudioProcessorValueTreeState &parameters,
                  juce::AudioProcessorValueTreeState &parametersNA,
                  zlInterface::UIBase &base,
                  zlDSP::Controller<double> &controller);

        ~SoloPanel() override;

        void paint(juce::Graphics &g) override;

    private:
        juce::AudioProcessorValueTreeState &parametersRef;
        zlInterface::UIBase &uiBase;
        zlIIR::Filter<double> &soloF;
        zlDSP::Controller<double> &controllerRef;
        std::atomic<float> scale1{.5f}, scale2{.5f};

        static constexpr std::array changeIDs{
            zlDSP::fType::ID, zlDSP::freq::ID, zlDSP::Q::ID,
            zlDSP::sideFreq::ID, zlDSP::sideQ::ID,
            zlDSP::solo::ID, zlDSP::sideSolo::ID
        };

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;
    };
} // zlPanel

#endif //ZLEqualizer_SOLO_PANEL_HPP
