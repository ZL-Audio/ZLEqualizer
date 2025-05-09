// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../state/state.hpp"
#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"
#include "../panel_definitons.hpp"

namespace zlpanel {
    class OutputValuePanel final : public juce::Component,
                                   private juce::MultiTimer,
                                   private juce::ValueTree::Listener {
    public:
        explicit OutputValuePanel(PluginProcessor &p,
                                  zlgui::UIBase &base);

        ~OutputValuePanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

    private:
        PluginProcessor &processorRef;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlgui::UIBase &uiBase;
        juce::RangedAudioParameter *lmPara;
        std::atomic<float> &scale;
        float currentGain{0.f}, currentScale{100.f};
        bool currentLearning{false};
        juce::String gainString{"0.0"}, scaleString{"100%"};
        bool showGain{false};
        juce::Rectangle<float> gainBound, scaleBound;
        juce::Path backgroundPath;

        void timerCallback(int timerID) override;

        void lookAndFeelChanged() override;

        void updateGainValue();

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override;
    };
} // zlpanel
