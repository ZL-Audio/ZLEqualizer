// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_OUTPUT_SETTING_PANEL_HPP
#define ZLEqualizer_OUTPUT_SETTING_PANEL_HPP

#include "../../state/state.hpp"
#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"
#include "../panel_definitons.hpp"

namespace zlPanel {
    class OutputSettingPanel final : public juce::Component,
                                     private juce::Timer {
    public:
        explicit OutputSettingPanel(PluginProcessor &p,
                                    zlInterface::UIBase &base);

        ~OutputSettingPanel() override;

        void paint(juce::Graphics &g) override;

        void mouseDown(const juce::MouseEvent &event) override;

    private:
        PluginProcessor &processorRef;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        juce::String displayString {"Output"};
        bool showGain{false};

        zlInterface::CallOutBoxLAF callOutBoxLAF;
        juce::Component::SafePointer<juce::CallOutBox> boxPointer;

        void openCallOutBox();

        void timerCallback() override;

        void lookAndFeelChanged() override;
    };
} // zlPanel

#endif //ZLEqualizer_OUTPUT_SETTING_PANEL_HPP
