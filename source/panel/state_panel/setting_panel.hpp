// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"

namespace zlPanel {
    class SettingPanel final : public juce::Component, private juce::MultiTimer {
    public:
        explicit SettingPanel(PluginProcessor &p, zlInterface::UIBase &base,
                              juce::String label, zlInterface::boxIdx idx);

        ~SettingPanel() override;

        void resized() override;

        void paint(juce::Graphics &g) override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseEnter(const juce::MouseEvent &event) override;

        void mouseExit(const juce::MouseEvent &event) override;

    private:
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        zlInterface::NameLookAndFeel nameLAF;
        juce::Label name;
        zlInterface::boxIdx mIdx;

        void timerCallback(int timerID) override;
    };
} // zlPanel
