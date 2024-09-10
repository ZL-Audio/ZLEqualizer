// Copyright (C) 2024 - zsliu98
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
#include "fft_panel/fft_panel.hpp"
#include "sum_panel/sum_panel.hpp"
#include "sum_panel/solo_panel.hpp"
#include "single_panel/single_panel.hpp"
#include "button_panel/button_panel.hpp"
#include "conflict_panel/conflict_panel.hpp"

namespace zlPanel {
    class CurvePanel final : public juce::Component,
                             private juce::AudioProcessorValueTreeState::Listener,
                             private juce::Thread {
    public:
        explicit CurvePanel(juce::AudioProcessorValueTreeState &parameters,
                            juce::AudioProcessorValueTreeState &parametersNA,
                            zlInterface::UIBase &base,
                            zlDSP::Controller<double> &c);

        ~CurvePanel() override;

        void paint(juce::Graphics &g) override;

        void paintOverChildren (juce::Graphics &g) override;

        void resized() override;

    private:
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        zlDSP::Controller<double> &controllerRef;
        std::array<zlFilter::Ideal<double, 16>, 16> baseFilters, targetFilters, mainFilters;
        BackgroundPanel backgroundPanel;
        FFTPanel fftPanel;
        ConflictPanel conflictPanel;
        SumPanel sumPanel;
        SoloPanel soloPanel;
        ButtonPanel buttonPanel;
        std::array<std::unique_ptr<SinglePanel>, zlState::bandNUM> singlePanels;
        juce::Time currentT;
        juce::VBlankAttachment vblank;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void repaintCallBack();

        void run() override;
    };
}


#endif //ZLEqualizer_CURVE_PANEL_HPP
