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
#include "../../dsp/dsp.hpp"
#include "background_panel/background_panel.hpp"
#include "fft_panel/fft_panel.hpp"
#include "sum_panel/sum_panel.hpp"
#include "sum_panel/solo_panel.hpp"
#include "single_panel/single_panel.hpp"
#include "single_panel/side_panel.hpp"
#include "button_panel/button_panel.hpp"
#include "conflict_panel/conflict_panel.hpp"
#include "match_panel/match_panel.hpp"
#include "loudness_display/loudness_display.hpp"

namespace zlpanel {
    class CurvePanel final : public juce::Component,
                             private juce::AudioProcessorValueTreeState::Listener,
                             private juce::ValueTree::Listener,
                             private juce::Thread {
    public:
        explicit CurvePanel(PluginProcessor &processor,
                            zlgui::UIBase &base);

        ~CurvePanel() override;

        void paint(juce::Graphics &g) override;

        void paintOverChildren(juce::Graphics &g) override;

        void resized() override;

        void repaintCallBack(double nowT);

    private:
        PluginProcessor &processor_ref_;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        zlp::Controller<double> &controller_ref_;
        std::array<zldsp::filter::Ideal<double, 16>, 16> baseFilters, targetFilters, mainFilters;
        BackgroundPanel backgroundPanel;
        FFTPanel fftPanel;
        ConflictPanel conflictPanel;
        SumPanel sumPanel;
        LoudnessDisplay loudnessDisplay;
        ButtonPanel buttonPanel;
        SoloPanel soloPanel;
        std::array<std::unique_ptr<SinglePanel>, zlstate::bandNUM> singlePanels;
        std::array<std::unique_ptr<SidePanel>, zlstate::bandNUM> sidePanels;
        juce::Component dummyComponent{};
        std::atomic<size_t> currentBandIdx;
        size_t previousBandIdx{zlstate::bandNUM + 1};
        MatchPanel matchPanel;
        double currentT{0.0};
        bool toNotify{false};

        std::atomic<bool> showMatchPanel{false};
        bool showUISettingsPanel{false};

        std::atomic<float> physicalPixelScaleFactor{1.f};

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override;

        void run() override;
    };
}
