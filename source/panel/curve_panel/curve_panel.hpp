// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_CURVE_PANEL_HPP
#define ZLEqualizer_CURVE_PANEL_HPP

#include "../../dsp/dsp.hpp"
#include "background_panel/background_panel.hpp"
#include "sum_panel/sum_panel.hpp"
#include "sum_panel/solo_panel.hpp"
#include "single_panel/single_panel.hpp"

namespace zlPanel {
    class CurvePanel final : public juce::Component,
    private juce::AudioProcessorValueTreeState::Listener{
    public:
        explicit CurvePanel(juce::AudioProcessorValueTreeState &parameters,
                            juce::AudioProcessorValueTreeState &parametersNA,
                            zlInterface::UIBase &base,
                            zlDSP::Controller<float> &c);

        ~CurvePanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;
    private:
        juce::AudioProcessorValueTreeState &parametersNARef;
        zlInterface::UIBase &uiBase;
        BackgroundPanel backgroundPanel;
        SumPanel sumPanel;
        SoloPanel soloPanel;
        std::array<std::unique_ptr<SinglePanel>, zlState::bandNUM> singlePanels;

        void parameterChanged(const juce::String &parameterID, float newValue) override;
    };
}


#endif //ZLEqualizer_CURVE_PANEL_HPP
