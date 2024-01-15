// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_SINGLE_PANEL_HPP
#define ZLEqualizer_SINGLE_PANEL_HPP

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../state/state_definitions.hpp"
#include "side_panel.hpp"

namespace zlPanel {
    class SinglePanel final : public juce::Component,
                              private juce::AudioProcessorValueTreeState::Listener,
                              private juce::AsyncUpdater {
    public:
        explicit SinglePanel(size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA,
                             zlInterface::UIBase &base,
                             zlDSP::Controller<double> &controller);

        ~SinglePanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void setMaximumDB(const float x) {
            maximumDB.store(x);
            triggerAsyncUpdate();
        }

    private:
        juce::Path path;

        size_t idx;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        std::atomic<bool> dynON, selected, actived;
        zlDSP::Controller<double> &controllerRef;
        zlDynamicFilter::IIRFilter<double> &filter;
        zlIIR::Filter<double> &baseF, &targetF;
        std::atomic<float> maximumDB;

        static constexpr std::array changeIDs{
            zlDSP::fType::ID, zlDSP::slope::ID,
            zlDSP::freq::ID, zlDSP::gain::ID, zlDSP::Q::ID,
            zlDSP::lrType::ID, zlDSP::dynamicON::ID,
            zlDSP::targetGain::ID, zlDSP::targetQ::ID
        };

        juce::Colour colour;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;

        void drawCurve(const std::array<double, zlIIR::frequencies.size()> &dBs, bool reverse = false,
                       bool startPath = true);

        SidePanel sidePanel;
    };
} // zlPanel

#endif //ZLEqualizer_SINGLE_PANEL_HPP